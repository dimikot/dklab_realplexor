#@
#@ Dklab Realplexor: Comet server which handles 1000000+ parallel browser connections
#@ Author: Dmitry Koterov, dkLab (C)
#@ GitHub: http://github.com/DmitryKoterov/
#@ Homepage: http://dklab.ru/lib/dklab_realplexor/
#@

##
## Server abstraction.
##
## Object of this class listens for incoming connection, accepts
## it and creates corresponding Realplexor::Event::Connection object.
##
package Realplexor::Event::Server;
use strict;
use Realplexor::Event::FH;
use IO::Socket::INET;
use EV;
use Carp;

# Static function.
# Runs the event mainloop.
sub mainloop {
    EV::loop();
}

# Now() function.
*now = \&EV::now;

# Creates a new server pool.
sub new {
    my ($class, %params) = @_;
    my $self = bless {
        %params,
        name    => ($params{name} or croak "Argument 'name' required"),
        listen  => ($params{listen} or croak "Argument 'listen' required"),
        timeout => (defined $params{timeout}? ($params{timeout} || undef) : croak "Argument 'timeout' required"),
        connectionclass => ($params{connectionclass} or croak "Argument 'connectionclass' required"),
        events => [],
    }, $class;
    my @events = ();
    my $lastAddr = undef;
    eval {
        foreach my $addr (@{$self->{listen}}) {
            $lastAddr = $addr;
            push @{$self->{events}}, $self->add_listen($addr);
        }
    };
    if ($@) {
        $self->{events} = [];
        croak(($lastAddr? "$lastAddr: " : "") . $@);
    }
    return $self;
}

# Adds a new listen address to the pool.
# Croaks in case of error.
sub add_listen {
    my ($self, $addr) = @_;
    my $sock = IO::Socket::INET->new(
        LocalAddr   => $addr,
        Proto       => 'tcp',
        ReuseAddr   => SO_REUSEADDR,
        Listen      => 50000,
        Blocking    => 1,
    ) or croak $@;
    my $event = EV::io $sock, EV::READ, sub {
        $self->handle_connect($sock);
    };
    $self->message(undef, "listening $addr");
    return $event;
}

# Called on a new connect.
sub handle_connect {
    my ($self, $sock) = @_;
    eval {
        my $accepted = $sock->accept() or die "accept failed: $@";
        $accepted->blocking(0); # this line is REALLY needed, else a hang may appear
        binmode($accepted);
        my $fh = new Realplexor::Event::FH($accepted);
        my $connection = $self->{connectionclass}->new($fh, $self);
        my $callback; $callback = sub {
            # Attention! $callback is a circular reference here!
            if ($self->handle_read($connection, $_[0])) {
                EV::once($accepted, EV::READ, $self->{timeout}, $callback);
            } else {
                # Break a circular reference in $callback when disconnected.
                $callback = undef;
            }
        };
        EV::once($accepted, EV::READ, $self->{timeout}, $callback);
    };
    $self->error($sock, $@) if $@;
}

# Called on data read.
sub handle_read {
    my ($self, $connection, $type) = @_;
    my $result = eval {
        # Timeout?
        if ($type & EV::TIMEOUT) {
            $connection->ontimeout();
            return 0;
        }

        # An error.
        if ($type & EV::ERROR) {
            $connection->onerror("An error returned to event handler");
            return 0;
        }

        # Read the next data chunk.
        my $nread = $connection->read_available_data();

        # End of the request reached (must never reach be cause of eof() check above?).
        if (!$nread) {
            return 0;
        }

        # Run data handler.
        $connection->onread($nread);
        return 1;
    };
    return $result if defined $result;
    if ($@) {
        $self->error($connection->fh, $@);
        return 0;
    }
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
        $addr = $addr->peeraddr;
    }
    $msg = $addr . ": " . $msg if $addr;
    $msg = $self->{name} . ": " . $msg;
    if (exists $self->{logger}) {
        $self->{logger}->($msg) if $self->{logger};
    } else {
        print "[" . localtime(now()) . "] $msg\n";
    }
}

return 1;
