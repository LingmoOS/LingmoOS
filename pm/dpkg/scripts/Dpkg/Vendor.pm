# Copyright © 2008-2009 Raphaël Hertzog <hertzog@debian.org>
# Copyright © 2008-2009, 2012-2017, 2022 Guillem Jover <guillem@debian.org>
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

Dpkg::Vendor - get access to some vendor specific information

=head1 DESCRIPTION

The files in $Dpkg::CONFDIR/origins/ can provide information about various
vendors who are providing Debian packages. Currently those files look like
this:

  Vendor: Debian
  Vendor-URL: https://www.debian.org/
  Bugs: debbugs://bugs.debian.org

If the vendor derives from another vendor, the file should document
the relationship by listing the base distribution in the Parent field:

  Parent: Debian

The file should be named according to the vendor name. The usual convention
is to name the vendor file using the vendor name in all lowercase, but some
variation is permitted. Namely, spaces are mapped to dashes ('-'), and the
file can have the same casing as the Vendor field, or it can be capitalized.

=cut

package Dpkg::Vendor 1.03;

use strict;
use warnings;
use feature qw(state);

our @EXPORT_OK = qw(
    get_current_vendor
    get_vendor_info
    get_vendor_file
    get_vendor_dir
    get_vendor_object
    run_vendor_hook
);

use Exporter qw(import);
use List::Util qw(uniq);

use Dpkg ();
use Dpkg::ErrorHandling;
use Dpkg::Gettext;
use Dpkg::BuildEnv;
use Dpkg::Control::HashCore;

my $origins = "$Dpkg::CONFDIR/origins";
$origins = $ENV{DPKG_ORIGINS_DIR} if $ENV{DPKG_ORIGINS_DIR};

=head1 FUNCTIONS

=over 4

=item $dir = get_vendor_dir()

Returns the current dpkg origins directory name, where the vendor files
are stored.

=cut

sub get_vendor_dir {
    return $origins;
}

=item $fields = get_vendor_info([$name])

Returns a L<Dpkg::Control> object with the information parsed from the
corresponding vendor file in $Dpkg::CONFDIR/origins/. If $name is omitted,
it will use $Dpkg::CONFDIR/origins/default which is supposed to be a symlink
to the vendor of the currently installed operating system. Returns undef
if there's no file for the given vendor.

=cut

my $vendor_sep_regex = qr{[^A-Za-z0-9]+};

sub get_vendor_info {
    my $vendor = shift || 'default';
    my $vendor_key = lc $vendor =~ s{$vendor_sep_regex}{}gr;
    state %VENDOR_CACHE;
    return $VENDOR_CACHE{$vendor_key} if exists $VENDOR_CACHE{$vendor_key};

    my $file = get_vendor_file($vendor);
    return unless $file;
    my $fields = Dpkg::Control::HashCore->new();
    $fields->load($file, compression => 0) or error(g_('%s is empty'), $file);
    $VENDOR_CACHE{$vendor_key} = $fields;
    return $fields;
}

=item $name = get_vendor_file([$name])

Check if there's a file for the given vendor and returns its
name.

The vendor filename will be derived from the vendor name, by replacing any
number of non-alphanumeric characters (that is B<[^A-Za-z0-9]>) into "B<->",
then the resulting name will be tried in sequence by lower-casing it,
keeping it as is, lower-casing then capitalizing it, and capitalizing it.

=cut

sub get_vendor_file {
    my $vendor = shift || 'default';

    my @names;
    my $vendor_sep = $vendor =~ s{$vendor_sep_regex}{-}gr;
    push @names, lc $vendor_sep, $vendor_sep, ucfirst lc $vendor_sep, ucfirst $vendor_sep;

    foreach my $name (uniq @names) {
        next unless -e "$origins/$name";
        return "$origins/$name";
    }
    return;
}

=item $name = get_current_vendor()

Returns the name of the current vendor. If DEB_VENDOR is set, it uses
that first, otherwise it falls back to parsing $Dpkg::CONFDIR/origins/default.
If that file doesn't exist, it returns undef.

=cut

sub get_current_vendor {
    my $f;
    if (Dpkg::BuildEnv::has('DEB_VENDOR')) {
        $f = get_vendor_info(Dpkg::BuildEnv::get('DEB_VENDOR'));
        return $f->{'Vendor'} if defined $f;
    }
    $f = get_vendor_info();
    return $f->{'Vendor'} if defined $f;
    return;
}

=item $object = get_vendor_object($name)

Return the Dpkg::Vendor::* object of the corresponding vendor.
If $name is omitted, return the object of the current vendor.
If no vendor can be identified, then return the L<Dpkg::Vendor::Default>
object.

The module name will be derived from the vendor name, by splitting parts
around groups of non alphanumeric character (that is B<[^A-Za-z0-9]>)
separators, by either capitalizing or lower-casing and capitalizing each part
and then joining them without the separators. So the expected casing is based
on the one from the B<Vendor> field in the F<origins> file.

=cut

sub get_vendor_object {
    my $vendor = shift || get_current_vendor() || 'Default';
    my $vendor_key = lc $vendor =~ s{$vendor_sep_regex}{}gr;
    state %OBJECT_CACHE;
    return $OBJECT_CACHE{$vendor_key} if exists $OBJECT_CACHE{$vendor_key};

    my @vendor_parts = split m{$vendor_sep_regex}, $vendor;

    my @names;
    push @names, join q{}, map { ucfirst } @vendor_parts;
    push @names, join q{}, map { ucfirst lc } @vendor_parts;

    foreach my $name (uniq @names) {
        my $module = "Dpkg::Vendor::$name";
        eval qq{
            require $module;
        };
        next if $@;

        my $obj = $module->new();
        $OBJECT_CACHE{$vendor_key} = $obj;
        return $obj;
    }

    my $info = get_vendor_info($vendor);
    if (defined $info and defined $info->{'Parent'}) {
        return get_vendor_object($info->{'Parent'});
    } else {
        return get_vendor_object('Default');
    }
}

=item run_vendor_hook($hookid, @params)

Run a hook implemented by the current vendor object.

=cut

sub run_vendor_hook {
    my @args = @_;
    my $vendor_obj = get_vendor_object();

    $vendor_obj->run_hook(@args);
}

=back

=head1 CHANGES

=head2 Version 1.03 (dpkg 1.22.12)

Obsolete behavior: get_vendor_file() and get_vendor_object() no longer
support the deprecated behavior from 1.02.

=head2 Version 1.02 (dpkg 1.21.10)

Deprecated behavior: get_vendor_file() loading vendor files with no special
characters remapping. get_vendor_object() loading vendor module names with
no special character stripping.

=head2 Version 1.01 (dpkg 1.17.0)

New function: get_vendor_dir().

=head2 Version 1.00 (dpkg 1.16.1)

Mark the module as public.

=head1 SEE ALSO

L<deb-origin(5)>.

=cut

1;
