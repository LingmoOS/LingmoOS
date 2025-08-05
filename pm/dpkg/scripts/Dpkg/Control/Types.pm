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

Dpkg::Control::Types - export CTRL_* constants

=head1 DESCRIPTION

You should not use this module directly. Instead you more likely
want to use L<Dpkg::Control> which also re-exports the same constants.

This module has been introduced solely to avoid a dependency loop
between L<Dpkg::Control> and L<Dpkg::Control::Fields>.

B<Note>: This is a private module, its API can change at any time.

=cut

package Dpkg::Control::Types 0.01;

use strict;
use warnings;

our @EXPORT = qw(
    CTRL_UNKNOWN
    CTRL_TMPL_SRC
    CTRL_TMPL_PKG
    CTRL_REPO_RELEASE
    CTRL_REPO_SRC
    CTRL_REPO_PKG
    CTRL_DSC
    CTRL_DEB
    CTRL_FILE_BUILDINFO
    CTRL_FILE_CHANGES
    CTRL_FILE_VENDOR
    CTRL_FILE_STATUS
    CTRL_CHANGELOG
    CTRL_COPYRIGHT_HEADER
    CTRL_COPYRIGHT_FILES
    CTRL_COPYRIGHT_LICENSE
    CTRL_TESTS

    CTRL_INFO_SRC
    CTRL_INFO_PKG
    CTRL_PKG_SRC
    CTRL_PKG_DEB
    CTRL_INDEX_SRC
    CTRL_INDEX_PKG
);

use Exporter qw(import);

use constant {
    CTRL_UNKNOWN => 0,
    # First source package control stanza in debian/control.
    CTRL_TMPL_SRC => 1 << 0,
    # Subsequent binary package control stanza in debian/control.
    CTRL_TMPL_PKG => 1 << 1,
    # Entry in repository's Sources files.
    CTRL_REPO_SRC => 1 << 2,
    # Entry in repository's Packages files.
    CTRL_REPO_PKG => 1 << 3,
    # .dsc file of source package.
    CTRL_DSC => 1 << 4,
    # DEBIAN/control in binary packages.
    CTRL_DEB => 1 << 5,
    # .changes file.
    CTRL_FILE_CHANGES => 1 << 6,
    # File in $Dpkg::CONFDIR/origins.
    CTRL_FILE_VENDOR => 1 << 7,
    # $Dpkg::ADMINDIR/status.
    CTRL_FILE_STATUS => 1 << 8,
    # Output of dpkg-parsechangelog.
    CTRL_CHANGELOG => 1 << 9,
    # Repository's (In)Release file.
    CTRL_REPO_RELEASE => 1 << 10,
    # Header control stanza in debian/copyright.
    CTRL_COPYRIGHT_HEADER => 1 << 11,
    # Files control stanza in debian/copyright.
    CTRL_COPYRIGHT_FILES => 1 << 12,
    # License control stanza in debian/copyright.
    CTRL_COPYRIGHT_LICENSE => 1 << 13,
    # Package test suite control file in debian/tests/control.
    CTRL_TESTS => 1 << 14,
    # .buildinfo file
    CTRL_FILE_BUILDINFO => 1 << 15,
};

# Backwards compatibility aliases.
use constant {
    CTRL_INFO_SRC => CTRL_TMPL_SRC,
    CTRL_INFO_PKG => CTRL_TMPL_PKG,
    CTRL_PKG_SRC => CTRL_DSC,
    CTRL_PKG_DEB => CTRL_DEB,
    CTRL_INDEX_SRC => CTRL_REPO_SRC,
    CTRL_INDEX_PKG => CTRL_REPO_PKG,
};

=head1 CHANGES

=head2 Version 0.xx

This is a private module.

=cut

1;
