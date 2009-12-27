#!/usr/bin/perl -w
##
## dklab_realplexor: simple and lightweight HTTP persistent
## connection emulation for JavaScript. Tool to handle 1000000+ 
## parallel browser connections.
##
## version 2.00
## (C) dkLab, http://dklab.ru/lib/dklab_realplexor/
## Changelog: http://github.com/DmitryKoterov/dklab_realplexor/commits/master/
##

use strict;
BEGIN {
	if (!eval('use Event::Lib; 1')) { 
		print STDERR "Error: Event::Lib library is not found in your system.\n";
		print STDERR "You must install libevent and Event::Lib, e.g.:\n";
		print STDERR "# yum install libevent-devel gcc\n"; 
		print STDERR "# perl -MCPAN -e \"install Event::Lib\"\n";
		print STDERR "  - or -\n"; 
		print STDERR "# apt-get install libevent-dev gcc\n"; 
		print STDERR "# perl -MCPAN -e \"install Event::Lib\"\n";
		print STDERR "(see http://monkey.org/~provos/libevent/ for details)\n";
		exit(1);
	}
}

use Storage::DataToSend;
use Storage::ConnectedFhs;
use Storage::OnlineTimers;
use Storage::PairsByFhs;
use Event::Lib::Connection;
use Event::Lib::Server;
use Connection::Wait;
use Connection::In;
use Realplexor::Config;
use Realplexor::Common;
use Getopt::Long;


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
	my $wait = Event::Lib::Server->new(
		name => "WAIT",
		listen => $CONFIG{WAIT_ADDR},
		timeout => $CONFIG{WAIT_TIMEOUT},
		connectionclass => "Connection::Wait",
		logger => \&Realplexor::Common::logger,
	);
	
	my $in = Event::Lib::Server->new(
		name => "IN",
		listen => $CONFIG{IN_ADDR},
		timeout => 0, #$CONFIG{IN_TIMEOUT},
		connectionclass => "Connection::In",
		logger => \&Realplexor::Common::logger,
	);

	# Catch signals.
	use POSIX qw(SIGHUP SIGINT);
	Event::Lib::Server::signal(SIGHUP, sub {
		Realplexor::Common::logger("SIGHUP received, reloading the config");
		my $low_level_opt = Realplexor::Config::reload($additional_conf);
		if ($low_level_opt) {
			Realplexor::Common::logger("Low-level option $low_level_opt is changed, restarting the script from scratch");
			exit();
		}
		return;
	});
	Event::Lib::Server::signal(SIGINT, sub {
		Realplexor::Common::logger("SIGINT received, exiting");
		exit();
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
	
	Event::Lib::Server::mainloop();
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
$SIG{PIPE} = sub { Realplexor::Common::logger("SIGPIPE received"); return 0; };

# Handle all signals to call END{} block at the end.
use sigtrap qw(die untrapped normal-signals);

# Parse command-line args.	
my @save_argv = @ARGV;
my $pid_file;
GetOptions("p=s" => \$pid_file);

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
	while (wait() != -1) {}
	sleep(1);
}

# Called if process dies.
END {
	return if !$pid; # children
	kill(2, $pid);
	sleep(1);
	if (kill(0, $pid)) {
		kill(9, $pid);
		print STDERR "Killed the child using a heavy SIGKILL.\n";
	} else {
		print STDERR "Normally terminated.\n";
	}
	if ($pid_file) {
		unlink($pid_file);
	}
}
