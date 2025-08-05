# Copyright © 2011 Raphaël Hertzog <hertzog@debian.org>
# Copyright © 2012 Guillem Jover <guillem@debian.org>
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

Dpkg::File - file handling

=head1 DESCRIPTION

This module provides file handling support functions.

B<Note>: This is a private module, its API can change at any time.

=cut

package Dpkg::File 0.01;

use strict;
use warnings;

our @EXPORT = qw(
    file_slurp
    file_dump
    file_touch
);

use Exporter qw(import);
use Scalar::Util qw(openhandle);

use Dpkg::ErrorHandling;
use Dpkg::Gettext;

sub file_slurp {
    my $file = shift;
    my $fh;
    my $doclose = 0;

    if (openhandle($file)) {
        $fh = $file;
    } else {
        open $fh, '<', $file or syserr(g_('cannot read %s'), $fh);
        $doclose = 1;
    }
    local $/;
    my $data = <$fh>;
    close $fh if $doclose;

    return $data;
}

sub file_dump {
    my ($file, $data) = @_;
    my $fh;
    my $doclose = 0;

    if (openhandle($file)) {
        $fh = $file;
    } else {
        open $fh, '>', $file or syserr(g_('cannot create file %s'), $file);
        $doclose = 1;
    }
    print { $fh } $data;
    if ($doclose) {
        close $fh or syserr(g_('cannot write %s'), $file);
    }

    return;
}

sub file_touch {
    my $file = shift;

    open my $fh, '>', $file or syserr(g_('cannot create file %s'), $file);
    close $fh or syserr(g_('cannot write %s'), $file);
}

=head1 CHANGES

=head2 Version 0.xx

This is a private module.

=cut

1;
