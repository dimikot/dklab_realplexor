package Tie::Array::Sorted;

use 5.006;

use strict;
use warnings;

use base 'Tie::Array';

our $VERSION = '1.41';

=head1 NAME

Tie::Array::Sorted - An array which is kept sorted

=head1 SYNOPSIS

	use Tie::Array::Sorted;

	tie @a, "Tie::Array::Sorted", sub { $_[0] <=> $_[1] };

	push @a, 10, 4, 7, 3, 4;
	print "@a"; # "3 4 4 7 10"

=head1 DESCRIPTION

This presents an ordinary array, but is kept sorted. All pushes and
unshifts cause the elements in question to be inserted in the
appropriate location to maintain order.

Direct stores (C<$a[10] = "wibble">) effectively splice out the original
value and insert the new element. It's not clear why you'd want to use
direct stores like that, but this module does the right thing if you do.

If you don't like the ordinary lexical comparator, you can provide your
own; it should compare the two elements it is given. For instance, a
numeric comparator would look like this:

	tie @a, "Tie::Array::Sorted", sub { $_[0] <=> $_[1] }

Whereas to compare a list of files by their sizes, you'd so something
like:

	tie @a, "Tie::Array::Sorted", sub { -s $_[0] <=> -s $_[1] }

=head1 LAZY SORTING

If you do more stores than fetches, you may find
L<Tie::Array::Sorted::Lazy> more efficient.

=cut

sub TIEARRAY {
	my ($class, $comparator) = @_;
	bless {
		array => [],
		comp  => (defined $comparator ? $comparator : sub { $_[0] cmp $_[1] })
	}, $class;
}

sub STORE {
	my ($self, $index, $elem) = @_;
	splice @{ $self->{array} }, $index, 0;
	$self->PUSH($elem);
}

sub PUSH {
	my ($self, @elems) = @_;
	ELEM: for my $elem (@elems) {
		my ($lo, $hi) = (0, $#{ $self->{array} });
		while ($hi >= $lo) {
			my $mid     = int(($lo + $hi) / 2);
			my $mid_val = $self->{array}[$mid];
			my $cmp     = $self->{comp}($elem, $mid_val);
			if ($cmp == 0) {
				splice(@{ $self->{array} }, $mid, 0, $elem);
				next ELEM;
			} elsif ($cmp > 0) {
				$lo = $mid + 1;
			} elsif ($cmp < 0) {
				$hi = $mid - 1;
			}
		}
		splice(@{ $self->{array} }, $lo, 0, $elem);
	}
}

sub UNSHIFT { goto &PUSH }

sub FETCHSIZE { scalar @{ $_[0]->{array} } }
sub STORESIZE { $#{ $_[0]->{array} } = $_[1] - 1 }
sub FETCH     { $_[0]->{array}->[ $_[1] ] }
sub CLEAR     { @{ $_[0]->{array} } = () }
sub POP       { pop(@{ $_[0]->{array} }) }
sub SHIFT     { shift(@{ $_[0]->{array} }) }

sub EXISTS { exists $_[0]->{array}->[ $_[1] ] }
sub DELETE { delete $_[0]->{array}->[ $_[1] ] }

1;

=head1 AUTHOR

Original author: Simon Cozens

Current maintainer: Tony Bowden

=head1 BUGS and QUERIES

Please direct all correspondence regarding this module to:
	bug-Tie-Array-Sorted@rt.cpan.org

This module was originall written as part of the L<Plucene> project.
However, as Plucene no longer uses this, it is effectively unmaintained.

=head1 COPYRIGHT AND LICENSE

  Copyright (C) 2003-2006 Simon Cozens and Tony Bowden.

  This program is free software; you can redistribute it and/or modify it under
  the terms of the GNU General Public License; either version 2 of the License,
  or (at your option) any later version.

  This program is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.


=cut

