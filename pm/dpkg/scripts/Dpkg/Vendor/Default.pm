# Copyright © 2009 Raphaël Hertzog <hertzog@debian.org>
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

Dpkg::Vendor::Default - default vendor class

=head1 DESCRIPTION

A vendor class is used to provide vendor specific behaviour
in various places. This is the default class used in case
there's none for the current vendor or in case the vendor could
not be identified (see L<Dpkg::Vendor> documentation).

It provides some hooks that are called by various dpkg-* tools.
If you need a new hook, please file a bug against dpkg-dev and explain
your need. Note that the hook API has no guarantee to be stable over an
extended period of time. If you run an important distribution that makes
use of vendor hooks, you'd better submit them for integration so that
we avoid breaking your code.

B<Note>: This is a private module, its API can change at any time.

=cut

package Dpkg::Vendor::Default 0.01;

use strict;
use warnings;

# If you use this file as template to create a new vendor class, please
# uncomment the following lines
#use parent qw(Dpkg::Vendor::Default);

=head1 METHODS

=over 4

=item $vendor_obj = Dpkg::Vendor::Default->new()

Creates the default vendor object. Can be inherited by all vendor objects
if they don't need any specific initialization at object creation time.

=cut

sub new {
    my $this = shift;
    my $class = ref($this) || $this;
    my $self = {};
    bless $self, $class;
    return $self;
}

=item $vendor_obj->run_hook($id, @params)

Run the corresponding hook. The parameters are hook-specific. The
supported hooks are:

=over 8

=item before-source-build ($srcpkg)

The first parameter is a L<Dpkg::Source::Package> object. The hook is called
just before the execution of $srcpkg->build().

=item package-keyrings ()

The hook is called when dpkg-source is checking a signature on a source
package (since dpkg 1.18.11). It takes no parameters, but returns a
(possibly empty) list of vendor-specific keyrings.

=item archive-keyrings ()

The hook is called when there is a need to check signatures on artifacts
from repositories, for example by a download method (since dpkg 1.18.11).
It takes no parameters, but returns a (possibly empty) list of
vendor-specific keyrings.

=item archive-keyrings-historic ()

The hook is called when there is a need to check signatures on artifacts
from historic repositories, for example by a download method
(since dpkg 1.18.11). It takes no parameters, but returns a (possibly empty)
list of vendor-specific keyrings.

=item builtin-build-depends ()

The hook is called when dpkg-checkbuilddeps is initializing the source
package build dependencies (since dpkg 1.18.2). It takes no parameters,
but returns a (possibly empty) list of vendor-specific B<Build-Depends>.

=item builtin-build-conflicts ()

The hook is called when dpkg-checkbuilddeps is initializing the source
package build conflicts (since dpkg 1.18.2). It takes no parameters,
but returns a (possibly empty) list of vendor-specific B<Build-Conflicts>.

=item register-custom-fields ()

The hook is called in L<Dpkg::Control::Fields> to register custom fields.
You should return a list of arrays. Each array is an operation to perform.
The first item is the name of the operation and corresponds
to a field_* function provided by L<Dpkg::Control::Fields>. The remaining
fields are the parameters that are passed unchanged to the corresponding
function.

Known operations are "register", "insert_after" and "insert_before".

=item post-process-changelog-entry ($fields)

The hook is called in L<Dpkg::Changelog> to post-process a
L<Dpkg::Changelog::Entry> after it has been created and filled with the
appropriate values.

=item update-buildflags ($flags)

The hook is called in L<Dpkg::BuildFlags> to allow the vendor to override
the default values set for the various build flags. $flags is a
L<Dpkg::BuildFlags> object.

=item builtin-system-build-paths ()

The hook is called by dpkg-genbuildinfo to determine if the current path
should be recorded in the B<Build-Path> field (since dpkg 1.18.11). It takes
no parameters, but returns a (possibly empty) list of root paths considered
acceptable. As an example, if the list contains "/build/", a Build-Path
field will be created if the current directory is "/build/dpkg-1.18.0". If
the list contains "/", the path will always be recorded. If the list is
empty, the current path will never be recorded.

=item build-tainted-by ()

The hook is called by dpkg-genbuildinfo to determine if the current system
has been tainted in some way that could affect the resulting build, which
will be recorded in the B<Build-Tainted-By> field (since dpkg 1.19.5). It
takes no parameters, but returns a (possibly empty) list of tainted reason
tags (formed by alphanumeric and dash characters).

=item sanitize-environment ()

The hook is called by dpkg-buildpackage to sanitize its build environment
(since dpkg 1.20.0).

=item backport-version-regex ()

The hook is called by dpkg-genchanges and dpkg-mergechangelog to determine
the backport version string that should be specially handled as not an earlier
than version or remapped so that it does not get considered as a pre-release
(since dpkg 1.21.3).
The returned string is a regex with one capture group for the backport
delimiter string, or undef if there is no regex.

=back

=cut

sub run_hook {
    my ($self, $hook, @params) = @_;

    if ($hook eq 'before-source-build') {
        my $srcpkg = shift @params;
    } elsif ($hook eq 'package-keyrings') {
        return ();
    } elsif ($hook eq 'archive-keyrings') {
        return ();
    } elsif ($hook eq 'archive-keyrings-historic') {
        return ();
    } elsif ($hook eq 'register-custom-fields') {
        return ();
    } elsif ($hook eq 'builtin-build-depends') {
        return ();
    } elsif ($hook eq 'builtin-build-conflicts') {
        return ();
    } elsif ($hook eq 'post-process-changelog-entry') {
        my $fields = shift @params;
    } elsif ($hook eq 'extend-patch-header') {
	my ($textref, $ch_info) = @params;
    } elsif ($hook eq 'update-buildflags') {
	my $flags = shift @params;
    } elsif ($hook eq 'builtin-system-build-paths') {
        return ();
    } elsif ($hook eq 'build-tainted-by') {
        return ();
    } elsif ($hook eq 'sanitize-environment') {
        return;
    } elsif ($hook eq 'backport-version-regex') {
        return;
    }

    # Default return value for unknown/unimplemented hooks
    return;
}

=item $vendor->set_build_features($flags)

Sets the vendor build features, which will then be used to initialize the
build flags.

=cut

sub set_build_features {
    my ($self, $flags) = @_;

    return;
}

=item $vendor->add_build_flags($flags)

Adds the vendor build flags to the compiler flag variables based on the
vendor defaults and previously set build features.

=cut

sub add_build_flags {
    my ($self, $flags) = @_;

    return;
}

=back

=head1 CHANGES

=head2 Version 0.xx

This is a private module.

=cut

1;
