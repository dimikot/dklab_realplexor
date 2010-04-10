#!/usr/bin/perl -w
##
## dklab_realplexor: simple and lightweight HTTP persistent
## connection emulation for JavaScript. Tool to handle 1000000+ 
## parallel browser connections.
##
## version 2.x
## (C) dkLab, http://dklab.ru/lib/dklab_realplexor/
## Changelog: http://github.com/DmitryKoterov/dklab_realplexor/commits/master/
##

use strict;
BEGIN {
	if (!eval('use EV; 1')) { 
		print STDERR "Error: EV library is not found in your system:\n";
		print STDERR "http://search.cpan.org/~mlehmann/EV-3.9/EV.pm\n";
		print STDERR "You must install EV, e.g.:\n";
		print STDERR "# yum install gcc\n"; 
		print STDERR "# perl -MCPAN -e \"install EV\"\n";
		print STDERR "  - or -\n"; 
		print STDERR "# apt-get install gcc\n"; 
		print STDERR "# perl -MCPAN -e \"install EV\"\n";
		exit(1);
	}
}

use Storage::DataToSend;
use Storage::ConnectedFhs;
use Storage::OnlineTimers;
use Storage::PairsByFhs;
use Realplexor::Event::Connection;
use Realplexor::Event::Server;
use Connection::Wait;
use Connection::In;
use Realplexor::Config;
use Realplexor::Common;
use Getopt::Long;
use Realplexor::Event::Signal;


# Main processing loop.
sub mainloop {
	my $ulimit = `/bin/sh -c "ulimit -n"`; chomp $ulimit;
	Realplexor::Common::logger("Starting. Opened files limit (ulimit -n): $ulimit.");

	# Load configs.
	my $additional_conf = $ARGV[0];
	Realplexor::Config::load($additional_conf);
	
	# Turn on STDOUT buffering in non-verbose mode.
	$| = 0 if $CONFIG{VERBOSITY} < 3;

	# Initialize servers.
	my $wait = Realplexor::Event::Server->new(
		name => "WAIT",
		listen => $CONFIG{WAIT_ADDR},
		timeout => $CONFIG{WAIT_TIMEOUT},
		connectionclass => "Connection::Wait",
		logger => \&Realplexor::Common::logger,
	);
	
	my $in = Realplexor::Event::Server->new(
		name => "IN",
		listen => $CONFIG{IN_ADDR},
		timeout => 0, #$CONFIG{IN_TIMEOUT},
		connectionclass => "Connection::In",
		logger => \&Realplexor::Common::logger,
	);

	# Catch signals.
	use POSIX qw(SIGHUP SIGINT SIGPIPE);
	Realplexor::Event::Signal::create(SIGHUP, sub {
		Realplexor::Common::logger("SIGHUP received, reloading the config");
		my $low_level_opt = Realplexor::Config::reload($additional_conf);
		if ($low_level_opt) {
			Realplexor::Common::logger("Low-level option $low_level_opt is changed, restarting the script from scratch");
			exit();
		}
		return;
	});
	Realplexor::Event::Signal::create(SIGINT, sub {
		Realplexor::Common::logger("SIGINT received, exiting");
		exit();
	});
	Realplexor::Event::Signal::create(SIGPIPE, sub {
		Realplexor::Common::logger("SIGPIPE ignored");
		return 0;
	});
	
	# If running as root, SU to safe user.
	my $user = $CONFIG{SU_USER};
	if (!$> && $user) {
		my ($implemented,$name,$passwd,$uid,$gid,$quota,$comment,$gcos,$dir,$shell,$expire) = eval { (1, getpwnam($user)) };
		if ($implemented) {
			die "User $user is not found\n" if !defined $name;
			Realplexor::Common::logger("Switching current user to unprivileged \"$user\"");
			$) = $gid; $( = $gid;
			$> = $uid; $< = $uid;
		}
	}
	
	Realplexor::Event::Server::mainloop();
}

# Re-run self with high ulimit.
Realplexor::Tools::rerun_unlimited();

# Chdir to script's directory.
use FindBin qw($Bin);
chdir($Bin);

# Turn off buffering in debug log. (Load test says that there is 
# no difference in performance if bufferring is turned off or not.)
$| = 1;

# Turn on zombie auto-reaper.
$SIG{CHLD} = 'IGNORE';

# Handle all signals to call END{} block at the end.
use sigtrap qw(die untrapped normal-signals);

# Parse command-line args.	
my @save_argv = @ARGV;
my $pid_file;
GetOptions("p=s" => \$pid_file);

# Load config: it is also needed by parent watchdog.
my $additional_conf = $ARGV[0];
Realplexor::Config::load($additional_conf, 1);

# Save PID?
if ($pid_file) {
	open(local *F, ">", $pid_file) or die "Cannot create $pid_file: $!\n";
	print F $$;
	close(F);
}	

# Child PID.
my $pid;

# Run watchdog loop.
while (1) {
	delete $SIG{HUP};
	$pid = fork();
	
	if (!defined $pid) {
		# Fork failed.
		Realplexor::Common::logger("fork() failed: $!");
	} elsif (!$pid) {
		# Child process.
		eval {
			@ARGV = @save_argv; # because GetOptions() modifies ARGV
			mainloop();
		};
		die $@ if $@;
		exit();
	}

	# Pass SIGHUP to children.
	$SIG{HUP} = sub {
		kill 1, $pid;
	};
	
	# Waid for child termination.
	Realplexor::Tools::wait_pid_with_memory_limit($pid, $CONFIG{MAX_MEM_MB});
	sleep(1);
}

# Called if process dies.
END {
	return if !$pid; # children
	Realplexor::Tools::graceful_kill($pid, $pid_file);
}
