#@
#@ Dklab Realplexor: Comet server which handles 1000000+ parallel browser connections
#@ Author: Dmitry Koterov, dkLab (C)
#@ GitHub: http://github.com/DmitryKoterov/
#@ Homepage: http://dklab.ru/lib/dklab_realplexor/
#@

##
## Fiber Handle: an opened socket (connection) with event-driven write buffering support.
##
## The task is to allow unlimited amount of data to be written to a client
## connection, even if this amount is larger than Unix socket buffers
## (by default in Perl this buffer is near 160K, in C++ it's near 2K).
##
package Realplexor::Event::FH;
use strict;
use EV;

# Creates a new write buffering socket.
sub new {
    my ($class, $sock) = @_;
    my $self = bless {
        sock  => $sock,
        addr  => ($sock->peerhost||'?') . ":" . ($sock->peerport||'?'),
    }, $class;
    return $self;
}

sub recv_and_append_to {
    my ($self) = @_;
    my $nread = 0;
    while (1) {
        $self->{sock}->recv(my $data, 1024);
        # End of the request reached (must never reach be cause of eof() check above?).
        if (!$nread && !defined $data) {
            return 0;
        }
        if (!defined $data || !length($data)) {
            last;
        }
        $nread += length($data);
        $_[1] .= $data;
    }
    return $nread;
}

sub send {
    my ($self, $data) = @_;
    return $self->{sock}->send($data);
}

sub shutdown {
    my ($self, $how) = @_;
    return $self->{sock}->shutdown($how);
}

sub peeraddr {
    my ($self) = @_;
    return $self->{addr};
}

sub close {
    my ($self) = @_;
    return $self->{sock}->close();
}

return 1;
