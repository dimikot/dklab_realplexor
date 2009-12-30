##
## Connection abstraction.
##
## Object of this class is created when a connection is accepted
## and destroyed when its connection is closed. Each object
## represents a separated connection and may hold intermediate
## data collected while its connection is processed.
##
package Realplexor::Event::Connection;
use strict;

# Called on new connection.
# DO NOT save $event object here to avoid cyclic references!
sub new {
	my ($class, $fh, $server) = @_;
	my $self = bless {
		fh     => $fh,
		server => $server,
		# Save peer address now, because it may be inaccessible
		# in case of the manual socket shutdown.
		addr   => ($fh->peerhost||'?') . ":" . ($fh->peerport||'?'),
	}, $class;
	$self->debug("connection opened");
	return $self;
}

# Called on connection close.
sub DESTROY {
	my ($self) = @_;
	eval {
		$self->onclose();
	};
	if ($@) {
		$self->server->error($self->fh, $@);
	}
	$self->debug("connection closed");
}

# Called on timeout.
sub ontimeout {
	my ($self) = @_;
	$self->debug("timeout");
}

# Called on event exception.
sub onerror {
	my ($self, $msg) = @_;
	$self->debug("error: $msg");
}

# Called on data read.
sub onread {
	my ($self, $data) = @_;
	$self->debug("read " . length($data) . " bytes");
}

# Called on close.
sub onclose {
}

# Returns the socket.
sub fh {
	my ($self) = @_;
	return $self->{fh};
}

# Returns the server.
sub server {
	my ($self) = @_;
	return $self->{server};
}

# Returns this connection name.
sub name {
	return undef;
}

# Prints a debug message.
sub debug {
	my ($self, $msg) = @_;
	my $name = $self->name;
	$self->{server}->debug($self->{addr}, ($name? "[$name] " : "") . $msg);
}

return 1;
