#!/usr/bin/perl -w
use lib '../..';
use Realplexor::Tools;
Realplexor::Tools::rerun_unlimited();

use IO::Socket;
use threads;

$| = 1;

my $num = ($ARGV[0] || 10000);
my $nproc = ($ARGV[1] || 1);

print "Press Enter to create $num connections..."; scalar <STDIN>;

my (@pids, $p, $pid);
for ($p = 1; $p <= $nproc; $p++) {
	$pid = fork();
	if ($pid) {
		push @pids, $pid;
	} else {
		last;
	}
}

if ($pid) {
	# Parent process waits until all children are dead.
	$SIG{CHLD} = 'IGNORE';
	$SIG{INT} = sub { kill 2, @pids; exit; };
	while (@pids) {
		sleep(1);
		@pids = grep { kill(0, $_) } @pids;
	}

	exit();
}

# Child process code.
print "\n[PID=$$] Creating connections...";
my @sock = ();
for (my $i = 0; $i < $num / $nproc; $i++) {
	my $sock = IO::Socket::INET->new(
		PeerAddr => '127.0.0.1',
		PeerPort => '8088',
	);
	if (!$sock) {
		print "$i: $@\n\n";
		exit;
	}
	push @sock, $sock;
	my $prev = select($sock); $| = 1; select($prev);
	print $sock "test $i\n";
	print $sock "identifier=10:id$i\n";
	print STDERR ".";
	$sock->flush();
}
print "\n[PID=$$] All connections are established.\n";

if ($nproc == 1) {
	print "\nPress Enter to read all the responses";
	scalar <STDIN>;
	for (my $i = 0; $i < @sock; $i++) {
		my $sock = $sock[$i];
		while (<$sock>) {
			chomp;
			print "$p# [$i]: $_\n";
		}
		close($sock);
	}
} else {
	# Wait forever.
	sleep(10000) while 1;
}
