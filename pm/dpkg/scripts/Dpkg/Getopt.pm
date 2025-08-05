# Copyright © 2014 Guillem Jover <guillem@debian.org>
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

Dpkg::Getopt - option parsing handling

=head1 DESCRIPTION

This module provides helper functions for option parsing, and complements
the system Getopt::Long module.

B<Note>: This is a private module, its API can change at any time.

=cut

package Dpkg::Getopt 0.02;

use strict;
use warnings;

our @EXPORT = qw(
    normalize_options
);

use Exporter qw(import);

sub normalize_options
{
    my (%opts) = @_;
    my $norm = 1;
    my @args;

    @args = map {
        if ($norm and m/^(-[A-Za-z])(.+)$/) {
            ($1, $2)
        } elsif ($norm and m/^(--[A-Za-z-]+)=(.*)$/) {
            ($1, $2)
        } else {
            $norm = 0 if defined $opts{delim} and $_ eq $opts{delim};
            $_;
        }
    } @{$opts{args}};

    return @args;
}

=head1 CHANGES

=head2 Version 0.xx

This is a private module.

=cut

1;
