##
## Realplexor configuration.
##
package Realplexor::Config;
use strict;
use base 'Exporter';
use File::Basename;
use Cwd 'abs_path';
use POSIX 'strftime';
our @EXPORT = qw(%CONFIG);
our %CONFIG;

my $root = dirname(dirname(abs_path(__FILE__)));

#
# Additional auto-generated parameters:
# - IFRAME_CONTENT
# - IFRAME_TIME
#

# Load config.
sub load {
	my ($add, $silent) = @_;
	# Reset config.
	%CONFIG = ();
	# Read default config.
	do($root . "/dklab_realplexor.conf"); die $@ if $@;
	# Read custom config.
	if ($add) {
		if (-f $add) {
			Realplexor::Common::logger("CONFIG: appending configuration from $add") if !$silent;
			do($add); die $@ if $@;
		} else {
			Realplexor::Common::logger("CONFIG: file $add does not exist, skipping");
		}
	}
	# Create custom properties.
	foreach my $param ('IFRAME', 'SCRIPT') {
		# Build IFRAME data.
		my $fname = $CONFIG{"${param}_FILE"};
		$fname = $root . "/" . $fname if $fname !~ m{^/}s;
		open(local *F, $fname) or die "Cannot read $fname: $!\n";
		local $/;
		my $content = <F>;
		close(F);
		$content =~ s{\$([a-z]\w*)}{defined $CONFIG{$1}? $CONFIG{$1} : "undefined-$1"}sgei;
		# Assign additional parameters.
		$CONFIG{"${param}_CONTENT"} = $content;
		$CONFIG{"${param}_TIME"} = strftime("%a, %e %b %Y %H:%M:%S GMT", gmtime((stat $fname)[9]));
	}
	$CONFIG{USERS} = read_users($CONFIG{USERS_FILE});
	
}

# Read htpasswd-style users list.
sub read_users {
	my ($file) = @_;
	$file = $root . '/' . $file if $file !~ m{^/}s;
	open(local *F, $file) or die "Cannot open $file: $!\n";
	my %users = ();
	while (<F>) {
		s/#.*//sg;
		s/^\s+|\s+$//sg;
		next if !$_;
		my ($login, $pass) = split ":", $_, 2;
		next if !defined $pass;
		if (length($login) && $login !~ /^[a-z0-9]+$/is) {
			# Must not contain special characters like "_" and others.
			Realplexor::Common::logger("Warning: login \"$login\" is not alphanumeric, skipped.");
			next;
		}
		$users{$login} = $pass;
	}
	return \%users;
}

# Return 1 if we need hard restart.
sub reload {
	my ($add) = @_;
	my $lowlevel = qr/^(WAIT_ADDR|WAIT_TIMEOUT|IN_ADDIN_TIMEOUT|SU_.*)$/s;
	my $ignore = qr/^(HOOK_|.*_CONTENT)$/s;
	# Load new config.
	my %old = %CONFIG;
	if (!eval { load($add); return 1 }) {
		Realplexor::Common::logger("Error reloading config, continue with old settings: $@");
		%CONFIG = %old;
		return;
	}
	# Check which options are changed.
	while (my ($opt, $v) = each %CONFIG) {
		my $o = $old{$opt};
		my $v_old = join(", ", @{ref $o eq 'ARRAY' && $o  or  ref $o eq 'HASH' && [map { "\"$_=>$o->{$_}\"" } keys %$o]  or  [$o||""]});
		my $v_new = join(", ", @{ref $v eq 'ARRAY' && $v  or  ref $v eq 'HASH' && [map { "\"$_=>$v->{$_}\"" } keys %$v]  or  [$v||""]});
		if ($v_old ne $v_new) {
			return $opt if $opt =~ $lowlevel;
			next if $opt =~ $ignore || ref $v eq "CODE";
			Realplexor::Common::logger("Option $opt is changed: $v_old -> $v_new");
		}
	}
	return;
}

return 1;
