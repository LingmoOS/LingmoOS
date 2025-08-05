# Copyright © 2006 Frank Lichtenheld <djpig@debian.org>
# Copyright © 2007-2009, 2012-2013 Guillem Jover <guillem@debian.org>
# Copyright © 2007 Raphaël Hertzog <hertzog@debian.org>
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

Dpkg::Package - package properties handling

=head1 DESCRIPTION

This module provides functions to parse and validate package properties.

B<Note>: This is a private module, its API can change at any time.

=cut

package Dpkg::Package 0.01;

use strict;
use warnings;

our @EXPORT = qw(
    pkg_name_is_illegal

    get_source_name
    set_source_name
);

use Exporter qw(import);

use Dpkg::ErrorHandling;
use Dpkg::Gettext;

sub pkg_name_is_illegal {
    my $name = shift // '';

    if ($name eq '') {
        return g_('may not be empty string');
    }
    if ($name =~ m/[^-+.0-9a-z]/op) {
        return sprintf(g_("character '%s' not allowed"), ${^MATCH});
    }
    if ($name !~ m/^[0-9a-z]/o) {
        return g_('must start with an alphanumeric character');
    }

    return;
}

# XXX: Eventually the following functions should be moved as methods for
# Dpkg::Source::Package.

my $source_name;

sub get_source_name {
    return $source_name;
}

sub set_source_name {
    my $name = shift;

    my $err = pkg_name_is_illegal($name);
    error(g_("source package name '%s' is illegal: %s"), $name, $err) if $err;

    if (not defined $source_name) {
        $source_name = $name;
    } elsif ($name ne $source_name) {
        error(g_('source package has two conflicting values - %s and %s'),
              $source_name, $name);
    }
}

=head1 CHANGES

=head2 Version 0.xx

This is a private module.

=cut

1;
