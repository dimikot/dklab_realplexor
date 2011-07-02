#!/usr/bin/perl -w
use lib '../..';
use Realplexor::Tools;
Realplexor::Tools::rerun_unlimited();
use IO::Socket;
use Time::HiRes qw(time usleep);

my $cursor = 10;
my $num_ids = ($ARGV[0] || 10000);

$| = 1;
print "Press Enter to start sending...\n"; scalar <STDIN>;

my @sock = ();
for (my $i = 0; $i < $num_ids; $i++) {
	my $sock = IO::Socket::INET->new(
		PeerAddr => '127.0.0.1',
		PeerPort => '10010'
	);
	if (!$sock) {
		print "$i: $@\n\n";
		last;
	}
	print $sock "identifier=$cursor:id$i\n\n";
	print $sock "test $i\n";
	print ".";
	push @sock, $sock;
}

print "\nPress Enter to disconnect and apply commands...\n"; scalar <STDIN>;
