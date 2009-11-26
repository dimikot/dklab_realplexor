##
## Storage::OnlineTimers: timers for each ID to track online users.
##
## Structure: { ID => TimerEvent }
## Each $id has associated a timeout function. The $online_timers{$id} 
## is assigned with a new TimerEvent object on a client connect, but 
## this timer object is turned to count-down only when the client is 
## disconnected. If the client is still connected, timer object is assigned, 
## but is not activated.
##
package Storage::OnlineTimers;
use strict;
use base 'Exporter';
use Event::Lib;
our @EXPORT = qw($online_timers);
our $online_timers = new Storage::OnlineTimers();

sub new {
	my ($class) = @_;
	return bless {}, $class;
}

# Return true if we just assigned this timer, false if it was
# already assigned.
sub assign_stopped_timer_for_id {
	my ($this, $id, $callback) = @_;
	my $firstTime = 1;
	# Remove current timer if present.
	if ($this->{$id}) {
		$this->{$id}->remove();
		delete $this->{$id};
		$firstTime = 0;
	}
	# Create new stopped timer.
	$this->{$id} = timer_new(sub { 
		delete $this->{$id};
		$callback->();
	});
	return $firstTime;
}

sub start_timer_by_id {
	my ($this, $id, $timeout) = @_;
	if ($this->{$id}) {
		$this->{$id}->remove(); # needed to avoid multiple addition of the same timer
		$this->{$id}->add($timeout);
	}
}

sub get_num_items {
	my ($this) = @_;
	return scalar(keys %$this);
}

sub get_ids_ref {
	my ($this, $idRe) = @_;
	if (defined($idRe)) {
		return [grep { $_ =~ $idRe } sort keys %$this];
	} else {
		return [sort keys %$this];
	}
}

sub get_stats {
	my ($this) = @_;
	my @result = ();
	foreach my $id (sort keys %$this) {
		push @result, "$id => assigned";
	}
	return @result? join("\n", @result) . "\n" : "";
}

return 1;
