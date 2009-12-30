##
## Storage::PairsByFhs: list of IDs by FHs.
##
## Structure: { FH => [ [cursor1, id1], [cursor2, id2], ...] }
## Which IDs are registered in which FH's. This information is used to 
## implement listening on multiple IDs during a single connection.
##
package Storage::PairsByFhs;
use base 'Exporter';
use strict;
our @EXPORT = qw($pairs_by_fhs);
our $pairs_by_fhs = new Storage::PairsByFhs();

sub new {
	my ($class) = @_;
	return bless {}, $class;
}

sub set_pairs_for_fh {
	my ($this, $fh, $pairs) = @_;
	$this->{$fh} = $pairs;
}

sub remove_by_fh {
	my ($this, $fh) = @_;
	delete $this->{$fh};
}

sub get_pairs_by_fh {
#	my ($this, $fh) = @_;
	return $_[0]->{$_[1]};
}

sub get_num_items {
	my ($this) = @_;
	return scalar(keys %$this);
}

sub get_stats {
	my ($this) = @_;
	my @result = ();
	foreach my $fh (sort keys %$this) {
		push @result, "$fh => " . join(", ", map { $_->[0] . ":" . $_->[1] } @{$this->{$fh}});
	}
	my $result = @result? join("\n", @result) . "\n" : "";
	$result =~ s/IO::Socket::INET=GLOB//sg;
	return $result;
}

return 1;
