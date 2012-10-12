##
## Realplexor tools.
##
package Realplexor::Tools;
use strict;
use Time::HiRes;
use Math::BigFloat;
use POSIX ":sys_wait_h";

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
#	my $time = Time::HiRes::time();  # Using double instead of BigFloat increases performance a little: 256 -> 282 rps
	my $cycle = 1000;
	$time_counter++;
	$time_counter = 0 if $time_counter > $cycle;
	my $add = (1.0 / $cycle) * $time_counter * 0.000001;
	return $time + $add;
}

# Rerun the script unlimited.
sub rerun_unlimited {
	if (($ARGV[0]||'') ne "-") {
		my $ulim = "$^O" eq "darwin" ? "" : "ulimit -n 1048576; ";
		my $cmd = "/bin/sh -c '$ulim exec \"$^X\" \"$0\" - " . join(" ", map { '"' . $_ . '"' } @ARGV) . "'";
		exec($cmd) or die "Cannot exec $cmd: $!\n";
	} else {
		shift @ARGV;
	}
}

# Returns amount of used memory by pid (in megabytes).
sub get_memory_usage {
	my ($pid) = @_;
	my $h = "$^O" =~ /^(freebsd|darwin)$/ ? " | awk 'NR>1'" : "--no-headers";
	my $mem = `ps -p $pid -o rss $h`;
	return 0 if !$mem;
	$mem =~ s/\s+//sg;
	return $mem / 1024;
}

# Wait for a process termination.
# If the process limits memory usage, kills it.
sub wait_pid_with_memory_limit {
	my ($pid, $limit) = @_;
	while (waitpid($pid, WNOHANG) != -1) {
		sleep(1);
		my $mem = get_memory_usage($pid);
		if ($limit && int($mem) > int($limit)) {
			print STDERR sprintf "Daemon process uses %d MB of memory which is larger than %d MB. Killing...\n", $mem, $limit;
			graceful_kill($pid);
			last;
		}
	}
	
}

# Gracefully kills a process.
sub graceful_kill {
	my ($pid, $pid_file) = @_;
	kill(2, $pid);
	sleep(1);
	if (kill(0, $pid)) {
		kill(9, $pid);
		print STDERR "Killed the child using a heavy SIGKILL.\n";
	} else {
		print STDERR "Normally terminated.\n";
	}
	if ($pid_file) {
		unlink($pid_file);
	}
	$pid = undef;
}

return 1;
