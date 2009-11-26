#!/usr/bin/perl -w
use lib '../..';
use Realplexor::Tools;
use IO::Socket;
Realplexor::Tools::rerun_unlimited();

my $sock = IO::Socket::INET->new(
	PeerAddr => '127.0.0.1',
	PeerPort => '10010'
);
print $sock "ABC identifier=20:test\n\n";
print $sock "test!";
close($sock);

system('ab -c 100 -n 10000 "http://127.0.0.1:8088/?identifier=10:test"');
