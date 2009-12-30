##
## Timers abstraction.
##
package Realplexor::Event::Timer;
use strict;
use EV;

sub create {
	my ($callback) = @_;
	return EV::timer_ns(0, 0, $callback);
}

sub start {
	my ($timer, $timeout) = @_;
	$timer->set($timeout, 0);
	$timer->start();
}

sub remove {
	my ($timer) = @_;
	# Nothing: removed by destructor.
}

return 1;
