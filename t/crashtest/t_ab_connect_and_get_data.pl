#!/usr/bin/perl -w
use lib '../..';
use Realplexor::Tools;
use IO::Socket;
Realplexor::Tools::rerun_unlimited();

my $dir = "/tmp/nytprof";
mkdir $dir;

chdir("../..");
if (!fork()) {
	open(STDOUT, ">", "$dir/dklab_realplexor.log");
	system "rm -f $dir/*.out*";
	$ENV{PERL5OPT} = '-d:NYTProf';
	$ENV{NYTPROF} = "sigexit=int,hup:addpid=1:file=$dir/nytprof.out";
	exec "perl dklab_realplexor.pl -p $dir/dklab_realplexor.pid";
}
sleep 1;

my $idsSend = join(",", map { "20:test$_" } (1 .. 80));
my $idsRecv = join(",", map { "10:test$_" } (1 .. 40));

my $sock = IO::Socket::INET->new(
	PeerAddr => '127.0.0.1',
	PeerPort => '10010'
);
print $sock "ABC identifier=$idsSend\n\n";
print $sock ("test!\n" x 50);
close($sock);

if (0 == system("ab -c 1 -n 100 'http://127.0.0.1:8088/?identifier=$idsRecv'")) {
	killchild();
	sleep 2;
}

chdir($dir);
system("rm -rf nytprof/");
system("nytprofhtml -f `ls -S *.out* | head -n 1`");

sub killchild {
	kill(2, `cat $dir/dklab_realplexor.pid`)? print("Killed!\n") : print("Not killed!\n");
}
