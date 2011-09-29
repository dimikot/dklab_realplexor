##
## WAIT connection.
##
package Connection::Wait;
use base 'Realplexor::Event::Connection';
use strict;
use Storage::PairsByFhs; 
use Storage::ConnectedFhs; 
use Storage::OnlineTimers; 
use Storage::DataToSend;
use Realplexor::Config;
use Storage::Events;

my $profiler_started = 0;

# Called on a new connection.
sub new {
	my ($class, @args) = @_;
	
	# Start profiler only at a first WAIT connection.
	if (!$profiler_started && $ENV{NYTPROF}) {
		if ($ENV{NYTPROF} =~ /\bfile=([^:]+)/s) {
			DB::enable_profile("$1.real");
		}
	}
	$profiler_started = 1;
	
	my $self = $class->SUPER::new(@args);
	$self->{pairs} = undef;
	$self->{data} = "";
	return $self;
}

# Called when a data is available to read.
sub onread {
	my ($self, $data) = @_;
	$self->SUPER::onread($data);

	# Data must be ignored, identifier is already extracted.
	if (defined $self->{pairs}) {
		return;
	}
	# Append data.
	$self->{data} .= $data;

	# Try to extract IDs from the new data chunk.
	#print "----------\n" . $self->{data} . "\n---------------\n";
	my $pairs = Realplexor::Common::extract_pairs($self->{data});
	if (defined $pairs) {
		die "Empty identifier passed\n" if !@$pairs;

		# Check if we have special marker: IFRAME.
		if ($pairs->[0][1] eq $CONFIG{IFRAME_ID}) {
			$self->debug("IFRAME marker received, sending content");
			Realplexor::Common::send_static($self->fh, 'IFRAME', "text/html; charset=$CONFIG{CHARSET}");
			return;
		}
		# Check if we have special marker: SCRIPT.
		if ($pairs->[0][1] eq $CONFIG{SCRIPT_ID}) {
			$self->debug("SCRIPT marker received, sending content");
			Realplexor::Common::send_static($self->fh, 'SCRIPT', "text/javascript; charset=$CONFIG{CHARSET}");
			return;
		}
		
		# IDs are extracted. Send response headers immediately.
		# We send response AFTER reading IDs, because before 
		# this reading we don't know if a static page or 
		# a data was requested.
		my $fh = $self->fh;
		print $fh 
			"HTTP/1.1 200 OK\r\n" .
			"Connection: close\r\n" .
			"Cache-Control: no-store, no-cache, must-revalidate\r\n" .
			"Expires: Mon, 26 Jul 1997 05:00:00 GMT\r\n" .
			"Content-Type: text/javascript; charset=$CONFIG{CHARSET}\r\n\r\n" .
			" \r\n"; # this immediate space plus text/javascript hides XMLHttpRequest in FireBug console
		$fh->flush();
		
		# Ignore all other input from IN and register identifiers.
		$self->{pairs} = $pairs;
		$self->{data} = undef; # GC
		$pairs_by_fhs->set_pairs_for_fh($self->fh, $pairs);
		foreach my $pair (@$pairs) {
			my ($cursor, $id) = @$pair;
			$connected_fhs->add_to_id($id, $cursor, $self->fh);
			# Create new online timer, but do not start it - it is 
			# started at LAST connection close, later.
			my $firstTime = $online_timers->assign_stopped_timer_for_id($id, sub { 
				Realplexor::Common::logger("[$id] is now offline");
				$events->notify("offline", $id);
			});
			if ($firstTime) {
				# If above returned true, this ID was offline, but become online.
				$events->notify("online", $id);
			}
		}
		$self->debug("registered"); # ids are already in the debug line prefix
		# Try to send pendings.
		Realplexor::Common::send_pendings([map { $_->[1] } @$pairs]);
		return;
	}

	# Check for the data overflow.
	if (length($self->{data}) > $CONFIG{WAIT_MAXLEN}) {
		die "overflow (received " . length($data) . " bytes total)\n";
	}
}

# Called on timeout (send error message).
sub ontimeout {
	my ($self) = @_;
	my $fh = $self->fh;
	if ($fh) {
		$fh->flush();
		shutdown($fh, 2);
	}
	$self->SUPER::ontimeout();
}

# Called on client disconnect.
sub onclose {
	my ($self) = @_;
	my $pairs = $self->{pairs};
	if (defined $pairs) {
		foreach my $pair (@$pairs) {
			my ($cursor, $id) = @$pair;
			# Remove the client from all lists.
			$connected_fhs->del_from_id_by_fh($id, $self->fh);
			# Turn on offline timer if it was THE LAST connection.
			if (!$connected_fhs->get_num_fhs_by_id($id)) {
				$online_timers->start_timer_by_id($id, $CONFIG{OFFLINE_TIMEOUT});
			}
		}
	}
	$pairs_by_fhs->remove_by_fh($self->fh);
}

# Connection name is its ID.
sub name {
	my $pairs = $_[0]->{pairs};
	return $_[0]->{name} ||= (
		$pairs && @$pairs
		? ($pairs->[0][0] . ":" . $pairs->[0][1]) . (@$pairs > 1? ",(and " . (@$pairs - 1) . " more)" : "")
		: undef
	);
}

return 1;
