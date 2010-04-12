#!/usr/bin/perl -w
use lib '../..';
use Realplexor::Tools;
Realplexor::Tools::rerun_unlimited();

use IO::Socket;
use Time::HiRes qw(time usleep);

$| = 1;
print "Press Enter to start sending...\n"; scalar <STDIN>;

my $sock = IO::Socket::INET->new(
	PeerAddr => '127.0.0.1',
	PeerPort => '10010'
);
if (!$sock) {
	die("$@\n\n");
}
print $sock "identifier=" . join(",", map { "20:id0" } (1..($ARGV[0]||500))) . "\n\n";
print $sock join("\n", map { "test $_" } (1..200));
$sock->shutdown(1);
print "Reading the response...\n";
while (!eof($sock)) {
	print scalar(<$sock>);
	sleep(1);
}
print "Done.\n";
close $sock;
