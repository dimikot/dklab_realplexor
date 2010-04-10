##
## Storage::ConnectedFhs: connected clients.
##
## Structure: { ID => { fh1 => [cursor1, fh1], fh2 => [cursor2, fh2], ... } }
## Each ID may be listened in a number of connections. So, when a data
## for $id is arrived, it is pushed to all $connected_fds{$id} clients.
## We store stringified FH in keys for faster access.
##
package Storage::ConnectedFhs;
use strict;
use base 'Exporter';
our @EXPORT = qw($connected_fhs);
our $connected_fhs = new Storage::ConnectedFhs();

sub new {
	my ($class) = @_;
	return bless {}, $class;
}

sub add_to_id {
	my ($this, $id, $cursor, $fh) = @_;
	$this->{$id}{$fh} = [$cursor, $fh];
}

sub del_from_id_by_fh {
	my ($this, $id, $fh) = @_;
	delete $this->{$id}{$fh};
	delete $this->{$id} if !%{$this->{$id}};
}

sub get_hash_by_id {
#	my ($this, $id) = @_;
	return $_[0]->{$_[1]};
}

sub get_num_items {
	my ($this) = @_;
	return scalar(keys %$this);
}

sub get_num_fhs_by_id {
	my ($this, $id) = @_;
	return $this->{$id}? scalar(keys %{$this->{$id}}) : 0;
}

sub get_stats {
	my ($this) = @_;
	my @result = ();
	foreach my $id (sort keys %$this) {
		push @result, "$id => " . join(", ", sort keys %{$this->{$id}});
	}
	my $result = @result? join("\n", @result) . "\n" : "";
	$result =~ s/IO::Socket::INET=GLOB//sg;
	return $result;
}

return 1;
