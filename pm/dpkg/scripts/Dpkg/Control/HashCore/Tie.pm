# Copyright © 2007-2009 Raphaël Hertzog <hertzog@debian.org>
# Copyright © 2009, 2012-2019, 2021 Guillem Jover <guillem@debian.org>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

=encoding utf8

=head1 NAME

Dpkg::Control::HashCore::Tie - ties a Dpkg::Control::Hash object

=head1 DESCRIPTION

This module provides a class that is used to tie a hash.
It implements hash-like functions by normalizing the name of fields received
in keys (using Dpkg::Control::Fields::field_capitalize()).
It also stores the order in which fields have been added in order to be able
to dump them in the same order.
But the order information is stored in a parent object of type
L<Dpkg::Control>.

B<Note>: This is a private module, its API can change at any time.

=cut

package Dpkg::Control::HashCore::Tie 0.01;

use strict;
use warnings;

use Dpkg::Control::FieldsCore;

use Carp;
use Tie::Hash;
use parent -norequire, qw(Tie::ExtraHash);

# $self->[0] is the real hash
# $self->[1] is a reference to the hash contained by the parent object.
# This reference bypasses the top-level scalar reference of a
# Dpkg::Control::Hash, hence ensuring that reference gets DESTROYed
# properly.

=head1 FUNCTIONS

=over 4

=item Dpkg::Control::Hash->new($parent)

Return a reference to a tied hash implementing storage of simple
"field: value" mapping as used in many Debian-specific files.

=cut

sub new {
    my ($class, @args) = @_;
    my $hash = {};

    tie %{$hash}, $class, @args; ## no critic (Miscellanea::ProhibitTies)
    return $hash;
}

sub TIEHASH  {
    my ($class, $parent) = @_;

    croak 'parent object must be Dpkg::Control::Hash'
        if not $parent->isa('Dpkg::Control::HashCore') and
           not $parent->isa('Dpkg::Control::Hash');
    return bless [ {}, $$parent ], $class;
}

sub FETCH {
    my ($self, $key) = @_;

    $key = lc($key);
    return $self->[0]->{$key} if exists $self->[0]->{$key};
    return;
}

sub STORE {
    my ($self, $key, $value) = @_;

    $key = lc($key);
    if (not exists $self->[0]->{$key}) {
        push @{$self->[1]->{in_order}}, field_capitalize($key);
    }
    $self->[0]->{$key} = $value;
}

sub EXISTS {
    my ($self, $key) = @_;

    $key = lc($key);
    return exists $self->[0]->{$key};
}

sub DELETE {
    my ($self, $key) = @_;
    my $parent = $self->[1];
    my $in_order = $parent->{in_order};

    $key = lc($key);
    if (exists $self->[0]->{$key}) {
        delete $self->[0]->{$key};
        @{$in_order} = grep { lc ne $key } @{$in_order};
        return 1;
    } else {
        return 0;
    }
}

sub FIRSTKEY {
    my $self = shift;
    my $parent = $self->[1];

    foreach my $key (@{$parent->{in_order}}) {
        return $key if exists $self->[0]->{lc $key};
    }
}

sub NEXTKEY {
    my ($self, $prev) = @_;
    my $parent = $self->[1];
    my $found = 0;

    foreach my $key (@{$parent->{in_order}}) {
        if ($found) {
            return $key if exists $self->[0]->{lc $key};
        } else {
            $found = 1 if $key eq $prev;
        }
    }
    return;
}

=back

=head1 CHANGES

=head2 Version 0.xx

This is a private module.

=cut

1;
