#!/usr/bin/perl -w
use lib '../..';
use Realplexor::Tools;
Realplexor::Tools::rerun_unlimited();

use IO::Socket;
use Time::HiRes qw(time usleep);

$| = 1;
print "Press Enter to create connections...\n"; scalar <STDIN>;

my @sock = ();
for (my $i = 0; $i < ($ARGV[0] || 10000); $i++) {
	my $sock = IO::Socket::INET->new(
		PeerAddr => '127.0.0.1',
		PeerPort => '8088'
	);
	if (!$sock) {
		print "$i: $@\n\n";
		last;
	}
	push @sock, $sock;
	print $sock "test $i\n";
	print $sock "identifier=id$i\n";
	print ".";
#	usleep(1000);
}

print "\nPress Enter to abnormally disconnect all...\n"; scalar <STDIN>;

