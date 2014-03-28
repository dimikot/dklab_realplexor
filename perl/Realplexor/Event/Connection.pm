#@
#@ Dklab Realplexor: Comet server which handles 1000000+ parallel browser connections
#@ Author: Dmitry Koterov, dkLab (C)
#@ GitHub: http://github.com/DmitryKoterov/
#@ Homepage: http://dklab.ru/lib/dklab_realplexor/
#@

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
# Note that $server is needed here for logging purposes only (e.g. $server->error()).
sub new {
    my ($class, $fh, $server) = @_;
    my $self = bless {
        fh     => $fh,
        server => $server,
        rdata  => "",
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

# Reads available data chunk from fh and returns number of read bytes.
sub read_available_data {
    my ($self) = @_;
    return $self->fh->recv_and_append_to($self->{rdata});
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
    my ($self, $nread) = @_;
    $self->debug("read $nread bytes");
}

# Called on close.
sub onclose {
}

# Returns the socket.
sub fh {
    my ($self) = @_;
    return $self->{fh};
}

# Returns this connection name.
sub name {
    return undef;
}

# Prints a debug message.
sub debug {
    my ($self, $msg) = @_;
    my $name = $self->name;
    $self->{server}->debug($self->fh, ($name? "[$name] " : "") . $msg);
}

return 1;
