##
## Server abstraction.
##
## Object of this class listens for incoming connection, accepts
## it and creates corresponding Event::Lib::Connection object.
##
package Event::Lib::Server;
use strict;
use IO::Socket::INET;
use Event::Lib;
use Carp;
use Time::HiRes 'time';

# Static function.
# Runs the event mainloop.
sub mainloop {
	event_mainloop();
}

# Static function.
# Assigns signal handler.
sub signal {
	my ($type, $sub) = @_;
	my $signal = signal_new($type, $sub);
	$signal->add();
}

# Creates a new server pool.
sub new {
	my ($class, %params) = @_;
	my $self = bless {
		%params,
		name    => ($params{name} or croak "Argument 'name' required"),
		listen  => ($params{listen} or croak "Argument 'listen' required"),
		timeout => (defined $params{timeout}? $params{timeout} : croak "Argument 'timeout' required"),
		connectionclass => ($params{connectionclass} or croak "Argument 'connectionclass' required"),
	}, $class;
	my @events = ();
	my $lastAddr = undef;
	eval {
		foreach my $addr (@{$self->{listen}}) {
			$lastAddr = $addr;
			push @events, $self->add_listen($addr);
		}
	};
	if ($@) {
		$_->remove() foreach @events;
		croak(($lastAddr? "$lastAddr: " : "") . $@);
	}
	return $self;
}

# Adds a new listen address to the pool.
# Croaks in case of error.
sub add_listen {
	my ($self, $addr) = @_;
	my $server = IO::Socket::INET->new(
		LocalAddr   => $addr,
		Proto       => 'tcp',
		ReuseAddr   => SO_REUSEADDR,
		Listen      => 50000,
		Blocking    => 1,
	) or croak $@;
	my $event  = event_new(
		$server, EV_READ|EV_PERSIST, 
		\&handle_connect,
		$self
	);
	$event->add();
	$self->message(undef, "listening $addr");
	return $event;
}

# Called on a new connect.
sub handle_connect {
	my ($e, $type, $self) = @_;
	eval {
		my $socket = $e->fh->accept() or die "accept failed: $@";
		$socket->blocking(0);
		# Try to add an event.
		my $event = event_new($socket, EV_READ|EV_PERSIST, \&handle_read);
		$event->add($self->{timeout});
		# If we are here, event is successfully added. Assign error handler.
		my $connection = $self->{connectionclass}->new($socket, $self);
		$event->args($self, $connection);
		$event->except_handler(\&handle_except);
	};
	$self->error($e->fh, $@) if $@;
}

# Called on data read.
sub handle_read {
	my ($e, $type, $self, $connection) = @_;
	eval {
		my $h = $e->fh;
	
		# Timeout?
		if ($type == EV_TIMEOUT) {
			$connection->ontimeout();
			$e->remove();
			return;
		}
	
		# Read the next data chunk.
		local $/;
		my $data = <$h>;
			
		# End of the request reached.
		if (!defined $data) {
			$e->remove();
			return;
		}
	
		# Run data handler.
		$connection->onread($data);
	};
	if ($@) {
		$self->error($e->fh, $@);
		$e->remove();
	}
}

# Called on error.
sub handle_except {
	my ($e, $msg, $type, $self, $connection) = @_;
	eval {
		$connection->onerror($msg);
		$e->remove();
	};
	$self->error($e->fh, $@) if $@;
}

# Controls debug messages.
sub debug {
	my ($self, $fh, $msg) = @_;
	$self->message($fh, "DEBUG: $msg");
}

# Controls error messages.
sub error {
	my ($self, $fh, $msg) = @_;
	$self->message($fh, "ERROR: $msg");
}

# Controls info messages.
sub message {
	my ($self, $addr, $msg) = @_;
	chomp($msg);
	if (ref $addr) {
		$addr = ($addr->peerhost||'?') . ":" . ($addr->peerport||'?');
	}
	$msg = $addr . ": " . $msg if $addr;
	$msg = $self->{name} . ": " . $msg;
	if (exists $self->{logger}) {
		$self->{logger}->($msg) if $self->{logger};
	} else {
		print "[" . localtime(time) . "] $msg\n";
	}
}

return 1;
