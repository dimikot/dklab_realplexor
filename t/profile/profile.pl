#!/usr/bin/perl -w
use lib '../..';
use Realplexor::Tools;
use IO::Socket;
use Getopt::Long;
Realplexor::Tools::rerun_unlimited();

# Read arguments.
my $profile = 0;
my $ids_send = 80;
my $ids_recv = 40;
my $ids_match = undef;
my $concur = 1;
my $req = undef;
GetOptions(
	"profile"  => \$profile,
	"send=i"   => \$ids_send,
	"recv=i"   => \$ids_recv,
	"concur=i" => \$concur,
	"match=i"  => \$ids_match,
	"req=i"    => \$req,
);
$req ||= $profile? 200 : 1000;
$ids_match = $ids_recv if !defined $ids_match;

print "Running $req requests listening $ids_recv channels each with $ids_send channels pre-filled.\n";
print "This is a syntetic operation: in real life only a litle number of listening channels match.\n\n";

# Path to store temporarily data to.
my $dir = "/tmp/dklab_realplexor_profile";
mkdir $dir;

# Fork our realplexor daemon.
chdir("../..");
if (!fork()) {
	open(STDOUT, ">", "$dir/dklab_realplexor.log");
	system "rm -f $dir/*.out*";
	if ($profile) {
		$ENV{PERL5OPT} = '-d:NYTProf';
		$ENV{NYTPROF} = "sigexit=int,hup:addpid=1:file=$dir/nytprof.out";
	}
	exec "perl dklab_realplexor.pl -p $dir/dklab_realplexor.pid";
}
sleep 1;

my $idsSend = join(",", map { "20:test$_" } (1 .. $ids_send));
my $idsRecv = join(",", map { $_ <= $ids_match? "10:test$_" : "10:other$_" } (1 .. $ids_recv));

# Send data to channels.
my $sock = IO::Socket::INET->new(
	PeerAddr => '127.0.0.1',
	PeerPort => '10010'
);
print $sock "ABC identifier=$idsSend\n\n";
print $sock ("test!\n" x 50);
close($sock);

# Start load testing.
my $cmd = "ab -c $concur -n $req 'http://127.0.0.1:8088/?identifier=$idsRecv'";
print "# $cmd\n";
if (0 == system($cmd)) {
	killchild();
	sleep 2;
}

# Generate profiler HTML.
if ($profile) {
	chdir($dir);
	system("rm -rf nytprof/");
	system("nytprofhtml -f `ls -S *.out* | head -n 1`");
}

# Kills realplexor daemon.
sub killchild {
	kill(2, `cat $dir/dklab_realplexor.pid`)? print("Killed!\n") : print("Not killed!\n");
}
