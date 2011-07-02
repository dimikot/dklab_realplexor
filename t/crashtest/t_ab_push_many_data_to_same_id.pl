#!/usr/bin/perl -w
use lib '../..';
use Realplexor::Tools;
Realplexor::Tools::rerun_unlimited();
use IO::Socket;
use Time::HiRes qw(time usleep);

my $num_datas = $ARGV[0] || 1;
my $data_len = $ARGV[1] || 1024;
my $id = "id0";
my $cursor = 10;

$| = 1;
print "Press Enter to start sending...\n"; scalar <STDIN>;

for (my $i = 0; $i < $num_datas; $i++) {
	my $sock = IO::Socket::INET->new(
		PeerAddr => '127.0.0.1',
		PeerPort => '10010'
	);
	if (!$sock) {
		die("$@\n\n");
	}
	print $sock "identifier=$cursor:$id\n\n";
	print $sock join("", map { "x" } (1..$data_len));
	print ".";
}
print "\nDone. Press Enter to run ab...\n";
scalar(<STDIN>);

my $cmd = "ab -c 10 -n 10000 'http://127.0.0.1:8088/?identifier=1:$id'";
print "# $cmd\n";
system($cmd);

print "Press Enter to finish.\n";
scalar(<STDIN>);
