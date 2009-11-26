#!/usr/bin/perl -w
use lib '../..';
use Realplexor::Tools;
Realplexor::Tools::rerun_unlimited();

use IO::Socket;
use threads;

$| = 1;
print "Press Enter to create connections..."; scalar <STDIN>;

my $num = ($ARGV[0] || 10000);
my $nproc = 1;

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
	$SIG{CHLD} = 'IGNORE';
	$SIG{INT} = sub { kill 2, @pids; exit; };
	while (1) {
		sleep(1);
	}
}

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
	print $sock "identifier=10:$i\n";
	print STDERR ".";
}
print "?";
sleep(1000);

for (my $i = 0; $i < @sock; $i++) {
	my $sock = $sock[$i];
	while (<$sock>) {
		chomp;
		print "$p# [$i]: $_\n";
	}
}
