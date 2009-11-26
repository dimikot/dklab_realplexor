##
## Storage::Events: list of events.
##
## Structure: { chains => [ [ event1 = [ time, type, id ], event2, ...], [ event3, event4, ...], ... ] }
## Hold list of events. First chain contains more recent events
## than the second etc. First event in a chain is older than
## second event in that chain etc.
##
package Storage::Events;
use strict;
use base 'Exporter';
use Realplexor::Config;
use Realplexor::Tools;
our @EXPORT = qw($events);
our $events = new Storage::Events();

sub new {
	my ($class) = @_;
	my $self = bless {}, $class;
	$self->{chains} = [];
	push @{$self->{chains}}, [];
	return $self;
}

sub notify {
	my ($self, $event, $id) = @_;
	my $chain = $self->{chains}[@{$self->{chains}} - 1];
	# Keep no more than EVENT_CHAIN_LEN items in each chain.
	if (@$chain > $CONFIG{EVENT_CHAIN_LEN}) {
		$chain = [];
		# Newest chains are pushed to the end of list.
		push @{$self->{chains}}, $chain;
	}
	# Keep no more than 2 chains total.
	if (@{$self->{chains}} > 3) {
		# Oldest chains are removed from the list head.
		shift @{$self->{chains}};
	}
	# Add item.
	push @$chain, [ Realplexor::Tools::time_hi_res(), $event, $id ];
}

# Return events never than $from_cursor.
# Format: [ [ time, type, id ], ... ]
sub get_recent_events {
	my ($self, $from_cursor, $idRe) = @_;
	$from_cursor = 0 if $from_cursor !~ /^[\d.]+$/s;
	$from_cursor = new Math::BigFloat($from_cursor);
	my @events = ();
	my %seen = ();
	OUTER:
	# View most recent chains first.
	foreach my $chain (reverse @{$self->{chains}}) {
		# Iterate most recent events first.
		for (my $i = @$chain - 1; $i >= 0; $i--) {
			my $event = $chain->[$i];
			last OUTER if $event->[0] <= $from_cursor;
			if (!$seen{$event->[2]} && (!$idRe || $event->[2] =~ $idRe)) {
				push @events, $event;
				$seen{$event->[2]} = 1;
			}
		}
	}
	return [ reverse @events ];
}

sub get_num_items {
	my ($self) = @_;
	my $sum = 0;
	foreach (@{$self->{chains}}) {
		$sum += @$_;
	}
	return $sum;
}

return 1;
