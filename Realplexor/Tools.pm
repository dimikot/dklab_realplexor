##
## Realplexor tools.
##
package Realplexor::Tools;
use strict;
use Time::HiRes;
use Math::BigFloat;

# Counter to make the time unique.
my $time_counter = 0;

# Return HiRes time. It is guaranteed that two sequencial calls
# of this function always return different time, second > first.
#
# ATTENTION! This function returns Math::BigFloat value, because
# standard double is not enough to differ sequential calls of 
# this function.
sub time_hi_res {
	my $time = new Math::BigFloat(Time::HiRes::time());
	$time->precision(-14); # number of digits after "."
	my $cycle = 1000;
	$time_counter++;
	$time_counter = 0 if $time_counter > $cycle;
	my $add = (1.0 / $cycle) * $time_counter * 0.000001;
	return $time + $add;
}

# Rerun the script unlimited.
sub rerun_unlimited {
	if (($ARGV[0]||'') ne "-") {
		my $cmd = "/bin/sh -c 'ulimit -n 1048576; $^X \"$0\" - " . join(" ", map { '"' . $_ . '"' } @ARGV) . "'";
		exec($cmd) or die "Cannot exec $cmd: $!\n";
	} else {
		shift @ARGV;
	}
}

return 1;
