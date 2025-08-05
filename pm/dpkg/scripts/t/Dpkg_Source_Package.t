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
use Test::Dpkg qw(:paths :needs);

use File::Compare;

use Dpkg::ErrorHandling;
use Dpkg::OpenPGP::ErrorCodes;

test_needs_openpgp_backend();

plan tests => 6;

use_ok('Dpkg::Source::Package');

report_options(quiet_warnings => 1);

my $datadir = test_get_data_path();
my $tmpdir = test_get_temp_path();
my $ascfile;

my $p = Dpkg::Source::Package->new();

$ascfile = "$tmpdir/package_1.0.orig.tar.enoent";
is($p->armor_original_tarball_signature("$datadir/nonexistent", $ascfile),
   undef, 'no conversion of inexistent file');

$ascfile = "$tmpdir/package_1.0.orig.tar.sig2asc";
is($p->armor_original_tarball_signature("$datadir/package_1.0.orig.tar.sig", $ascfile),
   OPENPGP_OK, 'conversion from binary sig to armored asc');

ok(compare($ascfile, "$datadir/package_1.0.orig.tar.asc") == 0,
   'binary signature converted to OpenPGP ASCII Armor');

# Grab the output messages.
eval {
    $ascfile = "$tmpdir/package_1.0.orig.tar.asc2asc";
    is($p->armor_original_tarball_signature("$datadir/package_1.0.orig.tar.asc", $ascfile),
       OPENPGP_OK, 'copy instead of converting already armored input');
};

ok(compare($ascfile, "$datadir/package_1.0.orig.tar.asc") == 0,
   'OpenPGP ASCII Armor copied to destination');

# TODO: Add actual test cases.
