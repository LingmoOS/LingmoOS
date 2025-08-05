# Copyright © 2016-2022 Guillem Jover <guillem@debian.org>
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

Dpkg::BuildInfo - handle build information

=head1 DESCRIPTION

The Dpkg::BuildInfo module provides functions to handle the build
information.

=cut

package Dpkg::BuildInfo 1.00;

use strict;
use warnings;

our @EXPORT_OK = qw(
    get_build_env_allowed
);

use Exporter qw(import);

=head1 FUNCTIONS

=over 4

=item @envvars = get_build_env_allowed()

Get an array with the allowed list of environment variables that can affect
the build, but are still not privacy revealing.

=cut

my @env_allowed = (
    # Toolchain.
    qw(
        CC
        CPP
        CXX
        OBJC
        OBJCXX
        PC
        FC
        M2C
        AS
        LD
        AR
        RANLIB
        MAKE
        AWK
        LEX
        YACC
    ),
    # Toolchain flags.
    qw(
        ASFLAGS
        ASFLAGS_FOR_BUILD
        CFLAGS
        CFLAGS_FOR_BUILD
        CPPFLAGS
        CPPFLAGS_FOR_BUILD
        CXXFLAGS
        CXXFLAGS_FOR_BUILD
        OBJCFLAGS
        OBJCFLAGS_FOR_BUILD
        OBJCXXFLAGS
        OBJCXXFLAGS_FOR_BUILD
        DFLAGS
        DFLAGS_FOR_BUILD
        FFLAGS
        FFLAGS_FOR_BUILD
        LDFLAGS
        LDFLAGS_FOR_BUILD
        ARFLAGS
        MAKEFLAGS
        GNUMAKEFLAGS
    ),
    # Dynamic linker, see ld(1).
    qw(
        LD_LIBRARY_PATH
    ),
    # Locale, see locale(1).
    qw(
        LANG
        LC_ALL
        LC_CTYPE
        LC_NUMERIC
        LC_TIME
        LC_COLLATE
        LC_MONETARY
        LC_MESSAGES
        LC_PAPER
        LC_NAME
        LC_ADDRESS
        LC_TELEPHONE
        LC_MEASUREMENT
        LC_IDENTIFICATION
    ),
    # Build flags, see dpkg-buildpackage(1).
    qw(
        DEB_BUILD_OPTIONS
        DEB_BUILD_PROFILES
    ),
    # DEB_flag_{SET,STRIP,APPEND,PREPEND} will be recorded after being merged
    # with system config and user config.
    # See deb-vendor(1).
    qw(
        DEB_VENDOR
    ),
    # See dpkg(1).
    qw(
        DPKG_ROOT
        DPKG_ADMINDIR
    ),
    # See dpkg-architecture(1).
    qw(
        DPKG_DATADIR
    ),
    # See Dpkg::Vendor(3).
    qw(
        DPKG_ORIGINS_DIR
    ),
    # See dpkg-gensymbols(1).
    qw(
        DPKG_GENSYMBOLS_CHECK_LEVEL
    ),
    # See <https://reproducible-builds.org/specs/source-date-epoch/>.
    qw(
        SOURCE_DATE_EPOCH
    ),
);

sub get_build_env_allowed {
    return @env_allowed;
}

=back

=head1 CHANGES

=head2 Version 1.00 (dpkg 1.21.14)

Mark the module as public.

=cut

1;
