#!/usr/bin/perl
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

use strict;
use warnings;

use Test::More;
use Test::Dpkg qw(:needs :paths);

BEGIN {
    plan tests => 24;

    use_ok('Dpkg::Control');
    use_ok('Dpkg::Control::Info');
}

my $datadir = test_get_data_path();

sub parse_ctrl {
    my ($type, $path) = @_;

    my $ctrl = Dpkg::Control->new(type => $type);
    eval {
        $ctrl->load($path);
        1;
    } or return;

    return $ctrl;
}

sub parse_dsc {
    my $path = shift;

    return parse_ctrl(CTRL_DSC, $path);
}

my $c = Dpkg::Control::Info->new("$datadir/control-1");

my $io_data;
my $io;

open $io, '>', \$io_data or die "canno open io string\n";;

$c->output($io);
my $expected = 'Source: mysource
Empty-Field:
Long-Field: line1
 line 2 line 2 line 2
 .
   line 3 line 3 line 3
 .
 ..
 line 4
My-Field-One: myvalue1
My-Field-Two: myvalue2
Numeric-Field: 0

Package: mypackage1
Architecture: any
Depends: libc6

Package: mypackage2
Architecture: all
Depends: hello

Package: mypackage3
Architecture: all
Depends: hello
Description: short one
 long one
 very long one
';
is($io_data, $expected, "Dump of $datadir/control-1");

my $src = $c->get_source();
is($src, $c->[0], 'array representation of Dpkg::Control::Info 1/2');
is($src->{'numeric-field'}, '0', 'Numeric 0 value parsed correctly');
is($src->{'my-field-one'}, 'myvalue1', 'Access field through badly capitalized field name');
is($src->{'long-field'},
'line1
line 2 line 2 line 2

  line 3 line 3 line 3

.
line 4', 'Get multi-line field');
is($src->{'Empty-field'}, '', 'Get empty field');

my $pkg = $c->get_pkg_by_idx(1);
is($pkg, $c->[1], 'array representation of Dpkg::Control::Info 2/2');
is($pkg->{package}, 'mypackage1', 'Name of first package');

$pkg = $c->get_pkg_by_name('mypackage3');
is($pkg->{package}, 'mypackage3', 'Name of third package');
is($pkg->{Depends}, 'hello', 'Name of third package');

$pkg = $c->get_pkg_by_idx(2);
open $io, '>', \$io_data or die "canno open io string\n";;
$pkg->output($io);

is($io_data,
'Package: mypackage2
Architecture: all
Depends: hello
', "Dump of second binary package of $datadir/control-1");

# Check OpenPGP armored signatures in source control files

my $dsc;

$dsc = parse_dsc("$datadir/bogus-unsigned.dsc");
is($dsc, undef, 'Unsigned .dsc w/ OpenPGP armor');

$dsc = parse_dsc("$datadir/bogus-armor-no-sig.dsc");
is($dsc, undef, 'Signed .dsc w/ OpenPGP armor missing signature');

$dsc = parse_dsc("$datadir/bogus-armor-trail.dsc");
is($dsc, undef, 'Signed .dsc w/ bogus OpenPGP armor trailer');

$dsc = parse_dsc("$datadir/bogus-armor-inline.dsc");
is($dsc, undef, 'Signed .dsc w/ bogus OpenPGP inline armor');

$dsc = parse_dsc("$datadir/bogus-armor-formfeed.dsc");
is($dsc, undef, 'Signed .dsc w/ bogus OpenPGP armor line');

$dsc = parse_dsc("$datadir/bogus-armor-double.dsc");
ok(defined $dsc, 'Signed .dsc w/ two OpenPGP armor signatures');
is($dsc->{Source}, 'pass', 'Signed spaced .dsc package name');

$dsc = parse_dsc("$datadir/bogus-armor-spaces.dsc");
ok(defined $dsc, 'Signed .dsc w/ spaced OpenPGP armor');
is($dsc->{Source}, 'pass', 'Signed spaced .dsc package name');

$dsc = parse_dsc("$datadir/bogus-armor-nested.dsc");
ok(defined $dsc, 'Signed .dsc w/ nested OpenPGP armor');
is($dsc->{Source}, 'pass', 'Signed nested .dsc package name');
