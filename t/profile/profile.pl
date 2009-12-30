#!/usr/bin/perl -w
use lib '../..';
use Realplexor::Tools;
use IO::Socket;
use Getopt::Long;
use Cwd 'abs_path';
use File::Basename;
Realplexor::Tools::rerun_unlimited();

# Read arguments.
my $profile = 0;
my $filled_channels = 5000;  # how many unique channels to fill
my $channel_size = 30;       # how much data to push to each channel
my $ids_listen = 5;          # how many ids to listen
my $ids_match = undef;       # how many ids from listened must match a non-empty channel
my $recv_pack_size = undef;
my $concur = 1;
my $req = undef;
GetOptions(
	"profile"           => \$profile,
	"filled_channels=i" => \$filled_channels,
	"channel_size=i"    => \$channel_size,
	"ids_listen=i"      => \$ids_listen,
	"ids_match=i"       => \$ids_match,
	"recv_pack_size=i"  => \$recv_pack_size,
	"concur=i"          => \$concur,
	"req=i"             => \$req,
);
$req ||= $profile? 1000 : 2000;
$ids_match = $ids_listen if !defined $ids_match;
$recv_pack_size = $channel_size if !defined $recv_pack_size;

print "Running $req requests listening $ids_listen channels each with $channel_size channels pre-filled.\n";

# Path to store temporarily data to.
my $dir = "/tmp/dklab_realplexor_profile";
my $log = "$dir/dklab_realplexor.log";
mkdir $dir;
chmod(0777, $dir);

# Fork our realplexor daemon.

my $cwd = dirname(abs_path(__FILE__));
chdir("../..");
if (!fork()) {
	open(STDOUT, ">", $log);
	system "rm -f $dir/*.out*";
	if ($profile) {
		$ENV{PERL5OPT} = '-d:NYTProf';
		$ENV{NYTPROF} = "sigexit=int,hup:start=no:addpid=1:file=$dir/nytprof.out";
	}
	exec "perl dklab_realplexor.pl $cwd/dklab_realplexor.conf -p $dir/dklab_realplexor.pid";
}
sleep 1;

# Send data to channels.
for (my $i = 0; $i < $channel_size; $i++) {
	my $idsSend = join(",", map { ($i + 2) . ":testidentifier" . sprintf("%.10d", $_) } (0 .. ($filled_channels - 1)));
	if ($i < 2 || $i == $channel_size - 1) {
		print "# send: " . (length($idsSend) > 70? (substr($idsSend, 0, 40) . "..." . substr($idsSend, -20)) : $idsSend) . "\n";
	} elsif ($i == 2) {
		print "# ...\n";
	}
	my $data = 
		"DATA identifier=$idsSend\n\n" .
		("xxxxxxxxxx\n" x 20);
	my $sock = IO::Socket::INET->new(
		PeerAddr => '127.0.0.1',
		PeerPort => '10010'
	);
	print $sock $data;
	close($sock);
}

# Wait for stop log activity.
print "Waiting until all IN data is processed...\n";
my $log_size = -s $log;
sleep(1);
while (-s $log != $log_size) {
	$log_size = -s $log;
	sleep(1);
}


# Start load testing.
my $idsRecv = join(",", map { $_ <= $ids_match? ($channel_size - $recv_pack_size + 1) . ":testidentifier{*}" : "10:other$_" } (0 .. ($ids_listen - 1)));

for (my $i = 0; $i < 1; $i++) {
	my $cmd = "ab -R $filled_channels -c $concur -n $req 'http://127.0.0.1:8088/?identifier=$idsRecv'";
	print "# $cmd\n";
	if (0 == system($cmd)) {
		killchild();
		sleep 2;
	}
}

# Generate profiler HTML.
if ($profile) {
	chdir($dir);
	system("rm -rf nytprof/");
	system("nytprofhtml -f `ls -S *.out* | head -n 1`"); # sort by file size reverse
}

# Kills realplexor daemon.
sub killchild {
	kill(2, `cat $dir/dklab_realplexor.pid`)? print("Killed!\n") : print("Not killed!\n");
}
