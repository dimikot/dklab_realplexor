##
## IN connection.
##
package Connection::In;
use base 'Realplexor::Event::Connection';
use strict;
use POSIX '_exit';
use Storage::PairsByFhs; 
use Storage::ConnectedFhs; 
use Storage::OnlineTimers; 
use Storage::DataToSend;
use Storage::CleanupTimers;
use Storage::Events;
use Realplexor::Config;

# Called on a new connection.
sub new {
	my ($class, @args) = @_;
	my $self = $class->SUPER::new(@args);
	$self->{data} = "";
	$self->{pairs} = undef;
	$self->{limit_ids} = undef;
	$self->{cred} = undef;
	return $self;
}

# Called on timeout.
sub ontimeout {
	my ($self) = @_;
	$self->SUPER::ontimeout();
	$self->{pairs} = undef;
}

# Called on error.	
sub onerror {
	my ($self, $msg) = @_;
	$self->SUPER::onerror();
	$self->{pairs} = undef;
}	

# Called when a data is available to read.
sub onread {
	my ($self, $data) = @_;
	$self->SUPER::onread($data);

	# Append data.
	$self->{data} .= $data if defined $self->{data};

	# Try to extract ID from the new data chunk.
	if (!defined $self->{pairs}) {
		my ($pairs, $limit_ids, $cred) = Realplexor::Common::extract_pairs($self->{data}, 1);
		if (defined $pairs) {
			$self->{pairs} = $pairs;
			$self->{limit_ids} = $limit_ids;
			$self->{cred} = $cred;
			$self->debug(
				"parsed IDs" 
				. ($limit_ids? "; limiters are (" . join(", ", sort keys %$limit_ids) . ")" : "")
				. ($cred? "; login is \"" . $cred->[0] . "\"" : "")
			);
			$self->assert_auth();
		}
	}

	# Try to process cmd.
	$self->try_process_cmd() and return;

	# Check for the data overflow.
	if (length($self->{data}) > $CONFIG{IN_MAXLEN}) {
		die "overflow (received " . length($data) . " bytes total)\n";
	}
}

# Called on client side disconnect.
sub onclose {
	my ($self) = @_;
	# First, try to process cmd.
	$self->try_process_cmd(1) and return;
	# Then, try to send messages.
	$self->try_process_pairs() and return;
}

# Assert that authentication is OK.
sub assert_auth {
	my ($self) = @_;
	my $cred = $self->{cred};
	eval {
		if ($cred) {
			# Login + password are passed. Check credentials.
			my $login = $cred->[0];
			if (!defined $CONFIG{USERS}{$login}) {
				die "unknown login: $login\n";
			}
			my $pwd_hash = $CONFIG{USERS}{$login};
			if (crypt($cred->[1], $pwd_hash) ne $pwd_hash) {
				die "invalid password for login: $login\n";
			}
		} elsif (!defined $CONFIG{USERS}{""}) {
			# Guest access, but no guest account is found.
			die "access denied for guest user\n";
		}
	};
	if ($@) {
		$self->{pairs} = undef;
		$self->{data} = undef;
		$self->_send_response($@, "403 Access Deined");
		die $@;
	}
}

# Process aux commands (may be started from the beginning
# of the data of from the first \r\n\r\n part and finished
# always by \n).
sub try_process_cmd {
	my ($self, $finished_reading) = @_;
	$self->{data} or return 0;
	# Try to extract cmd.
	my $tail_re = $finished_reading? qr/\r?\n\r?\n|$/s : qr/\r?\n\r?\n/s;
	$self->{data} =~ m/(?: ^ | \r?\n\r?\n) (ONLINE|STATS|WATCH) (?:\s+ ([^\r\n]*) )? (?: $tail_re )/six or return 0;
	# Cmd extracted, process it.
	$self->{pairs} = undef;
	$self->{data} = undef;
	# Assert authorization.
	$self->assert_auth();
	my $cmd = uc $1;
	my $arg = $2;
	$self->debug("received aux command: $cmd" . (defined $arg && length($arg)? " $arg" : ""));
	shutdown($self->fh, 0); # stop reading
	my $method = "cmd_" . lc($cmd);
	$self->$method($arg);
	return 1;
}

# Try to process pairs.
sub try_process_pairs {
	my ($self) = @_;
	$self->{data} or return;
	my $pairs = $self->{pairs};
	if (defined $pairs) {
		# Clear headers from the data.
		my (undef, $data) = split /\r?\n\r?\n/s, $self->{data}, 2;
		if (!defined($data)) {
			$self->debug("passed empty HTTP body, ignored");
			$self->{data} = undef;
			return;
		}
		my $login = $self->{cred}? $self->{cred}[0] : undef;
		my @ids_to_process = ();
		foreach my $pair (@$pairs) {
			my ($cursor, $id) = @$pair;
			# Check if it is not own pair.
			if (defined $login && substr($id, 0, length($login) + 1) ne $login . "_") {
				$self->debug("skipping not owned [$id] for login $login");
				next;
			}
			# Add data to queue and set lifetime.
			push @ids_to_process, $id;
			$data_to_send->add_dataref_to_id($id, $cursor, \$data, $self->{limit_ids});
			my $timeout = $CONFIG{CLEAN_ID_AFTER};
			$cleanup_timers->start_timer_for_id($id, $timeout, sub {
				$data_to_send->clear_id($id); 
				Realplexor::Common::logger("[$id] cleaned, because no data is pushed within last $timeout seconds");
			});
		}
		# One debug message per connection.
		$self->debug("added data for [" . join(",", @ids_to_process). "]") if @ids_to_process;
		# Send pending data.
		Realplexor::Common::send_pendings(\@ids_to_process);
	}
}

# Run a sub asynchronously.
sub _async {
	my ($self, $sub) = @_;
	my $pid = fork();
	if (!defined $pid) {
		$self->debug("cannot fork: $!");
	} elsif ($pid > 0) {
		# Parent process detaches.
		# Do nothing here.
	} else {
		# Child process.
		$sub->();
		close($self->fh);
		# We MUST use _exit(0) to avoid destructor calls.
		_exit(0);
	}
	
}

# Convert space-delimited ID prefixes list to regexp.
sub _id_prefixes_to_re {
	my ($self, $id_prefixes) = @_;
	my $re = (sub {
		return undef if !defined $id_prefixes;
		$id_prefixes =~ s/^\s+|\s+$//sg;
		return undef if !$id_prefixes;
		my @prefixes = split /\s+/, $id_prefixes;
		my $login = $self->{cred}? $self->{cred}[0] : undef;
		@prefixes = grep { substr($_, 0, length($login) + 1) eq $login . "_" } @prefixes if defined $login;
		return undef if !@prefixes;
		return "^(?:" . join("|", map { "\Q$_" } @prefixes) . ")";
	})->();
	# In case of an empty regexp (which means "no check needed") and
	# if a login is presented, return never-matched regexp.
	if (!defined $re && $self->{cred}) {
		return "^!";
	}
	return $re;
}

# Command: fetch all online IDs.
sub cmd_online {
	my ($self, $id_prefixes) = @_;
	$self->_async(sub {
		my $rids = $online_timers->get_ids_ref($self->_id_prefixes_to_re($id_prefixes));
		$self->debug("sending " . scalar(@$rids) . " online identifiers");
		$self->_send_response(join("", map { "$_ " . $connected_fhs->get_num_fhs_by_id($_) . "\n" } @$rids));
	});
}


# Command: watch for clients online/offline status changes.
sub cmd_watch {
	my ($self, $arg) = @_;
	$arg = "" if !defined $arg;
	my ($cursor, $id_prefixes) = split /\s+/, $arg, 2;
	$cursor = "" if !defined $cursor || !length $cursor;
	$id_prefixes = "" if !defined $id_prefixes || !length $id_prefixes;
	my $list = $events->get_recent_events($cursor, $self->_id_prefixes_to_re($id_prefixes));
	$self->debug("sending " . @$list . " events");
	$self->_send_response(join "", map { $_->[1] . " " . $_->[0] . ":" . $_->[2] . "\n" } @$list);
}

# Command: dump debug statistics.
# This command is for internal debugging only.
sub cmd_stats {
	my ($self) = @_;
	return if $self->{cred};
	$self->debug("sending stats");
	$self->_send_response(
		"[data_to_send]\n" .
		$data_to_send->get_stats() .
		"\n[connected_fhs]\n" .
		$connected_fhs->get_stats() .
		"\n[online_timers]\n" .
		$online_timers->get_stats() .
		"\n[cleanup_timers]\n" .
		$cleanup_timers->get_stats() .
		"\n[pairs_by_fhs]\n" .
		$pairs_by_fhs->get_stats()
	);
}

# Send response anc close the connection.
sub _send_response {
	my ($self, $rdata, $code) = ($_[0], \$_[1], $_[2]);
	my $fh = $self->fh;
	print $fh "HTTP/1.0 " . ($code || "200 OK") . "\r\n";
	print $fh "Content-Type: text/plain\r\n";
	print $fh "Content-Length: " . length($$rdata) . "\r\n\r\n";
	print $fh $$rdata;
	$fh->flush(); # MUST be executed! else SIGPIPE may be issued
	shutdown($fh, 2);
	$self->{data} = undef;
}

return 1;
