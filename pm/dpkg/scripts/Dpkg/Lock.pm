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

Dpkg::Lock - file locking support

=head1 DESCRIPTION

This module implements locking functions used to support parallel builds.

B<Note>: This is a private module, its API can change at any time.

=cut

package Dpkg::Lock 0.01;

use strict;
use warnings;

our @EXPORT = qw(
    file_lock
);

use Exporter qw(import);
use Fcntl qw(:flock);

use Dpkg::Gettext;
use Dpkg::ErrorHandling;

sub file_lock {
    my ($fh, $filename) = @_;

    # A strict dependency on libfile-fcntllock-perl being it an XS module,
    # and dpkg-dev indirectly making use of it, makes building new perl
    # package which bump the perl ABI impossible as these packages cannot
    # be installed alongside.
    eval q{
        use File::FcntlLock;
    };
    if ($@) {
        # On Linux systems the flock() locks get converted to file-range
        # locks on NFS mounts.
        if ($^O ne 'linux') {
            warning(g_('File::FcntlLock not available; using flock which is not NFS-safe'));
        }
        flock($fh, LOCK_EX)
            or syserr(g_('failed to get a write lock on %s'), $filename);
    } else {
        eval q{
            my $fs = File::FcntlLock->new(l_type => F_WRLCK);
            $fs->lock($fh, F_SETLKW)
                or syserr(g_('failed to get a write lock on %s'), $filename);
        }
    }
}

=head1 CHANGES

=head2 Version 0.xx

This is a private module.

=cut

1;
