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
package Realplexor::Event::Signal;
use strict;
use EV;

my @signals = ();

# Static function.
# Assigns signal handler.
sub create {
    my ($type, $sub) = @_;
    my $signal = EV::signal($type, $sub);
    push @signals, $signal; # save ref
}

return 1;
