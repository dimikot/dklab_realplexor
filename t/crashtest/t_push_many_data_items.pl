#!/usr/bin/perl -w
use lib '../..';
use Realplexor::Tools;
Realplexor::Tools::rerun_unlimited();

use IO::Socket;
use Time::HiRes qw(time usleep);

$| = 1;
print "Press Enter to start sending...\n"; scalar <STDIN>;

my @sock = ();
for (my $i = 0; $i < ($ARGV[0] || 10000); $i++) {
	my $sock = IO::Socket::INET->new(
		PeerAddr => '127.0.0.1',
		PeerPort => '10010'
	);
	if (!$sock) {
		print "$i: $@\n\n";
		last;
	}
	push @sock, $sock;
	print $sock "test $i\n";
	print $sock "identifier=20:id$i\n";
	print ".";
}

print "Press Enter to disconnect and apply commands...\n"; scalar <STDIN>;
