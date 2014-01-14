#@
#@ Dklab Realplexor: Comet server which handles 1000000+ parallel browser connections
#@ Author: Dmitry Koterov, dkLab (C)
#@ GitHub: http://github.com/DmitryKoterov/
#@ Homepage: http://dklab.ru/lib/dklab_realplexor/
#@

##
## Storage::DataToSend: data to send by ID.
##
## Structure: { ID => [ [cursor1, \$data1, \%limit_ids], [$cursor2, \$data2, \%limit_ids], ...] }
## When a client with $id is connected, and $data_to_send{$id} is not
## empty, the data is sent to that client IF this client requested
## a data with "cursor" marker less than presented at $data_to_send{$id}.
## The hash holds REFERENCES to arrays, so we could distinguish the same
## data block on different IDs. References are very handy: Perl takes
## care about garbage collecting and ref count, and when the data block
## is removed from the last ID, it is removed from the memory automatically.
##
## Third element, %limit_ids, shows that this data could be sent to
## only those who also listens IDs from %limit_ids keys. This is used
## to control data visibility.
##
package Storage::DataToSend;
use base 'Exporter';
use strict;
use Tie::Array::Sorted;
our @EXPORT = qw($data_to_send);
our $data_to_send = new Storage::DataToSend();

sub new {
    my ($class) = @_;
    return bless {}, $class;
}

sub clear_id {
    my ($this, $id) = @_;
    delete $this->{$id};
}

sub add_dataref_to_id {
    my ($this, $id, $cursor, $rdata, $rlimit_ids) = @_;
    my $list = $this->{$id};
    if (!$list) {
        $list = $this->{$id} = [];
        tie @$list, "Tie::Array::Sorted", sub { $_[1][0] <=> $_[0][0] };
    }
    # In most cases new cursor is greater than the first array
    # element, so we may unshif it without re-sorting to speedup.
    unshift @$list, [$cursor, $rdata, $rlimit_ids];
}

sub get_data_by_id {
#   my ($this, $id) = @_;
    return $_[0]->{$_[1]};
}

sub get_num_items {
    my ($this) = @_;
    return scalar(keys %$this);
}

sub clean_old_data_for_id {
    my ($this, $id, $max_num) = @_;
    return if !$this->{$id} || @{$this->{$id}} <= $max_num;
    splice @{$this->{$id}}, $max_num; # old elements are at the end
}

sub get_stats {
    my ($this) = @_;
    my @result = ();
    foreach my $id (sort keys %$this) {
        my @pairs = ();
        foreach my $pair (@{$this->{$id}}) {
            push @pairs,
                "[$pair->[0]: " .
                length(${$pair->[1]}) . "b" .
                ($pair->[2]? ", limited by (" . join(", ", sort keys %{$pair->[2]}) . ")" : "").
                "]";
        }
        push @result, "$id => " . join(", ", @pairs);
    }
    my $result = @result? join("\n", @result) . "\n" : "";
    return $result;
}

return 1;
