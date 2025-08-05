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

use Test::More tests => 14;

BEGIN {
    use_ok('Dpkg::BuildEnv');
}

$ENV{DPKG_TEST_VAR_A} = 100;
$ENV{DPKG_TEST_VAR_B} = 200;
delete $ENV{DPKG_TEST_VAR_Z};

is(scalar Dpkg::BuildEnv::list_accessed(), 0, 'no accessed variables');
is(scalar Dpkg::BuildEnv::list_modified(), 0, 'no modified variables');

is(Dpkg::BuildEnv::get('DPKG_TEST_VAR_A'), 100, 'get value');

is(scalar Dpkg::BuildEnv::list_accessed(), 1, '1 accessed variables');
is(scalar Dpkg::BuildEnv::list_modified(), 0, 'no modified variables');

is(Dpkg::BuildEnv::get('DPKG_TEST_VAR_B'), 200, 'get value');
Dpkg::BuildEnv::set('DPKG_TEST_VAR_B', 300);
is(Dpkg::BuildEnv::get('DPKG_TEST_VAR_B'), 300, 'set value');

is(scalar Dpkg::BuildEnv::list_accessed(), 2, '2 accessed variables');
is(scalar Dpkg::BuildEnv::list_modified(), 1, '1 modified variable');

ok(Dpkg::BuildEnv::has('DPKG_TEST_VAR_A'), 'variables is present');
ok(!Dpkg::BuildEnv::has('DPKG_TEST_VAR_Z'), 'variables is not present');

is(scalar Dpkg::BuildEnv::list_accessed(), 3, '2 accessed variables');
is(scalar Dpkg::BuildEnv::list_modified(), 1, '1 modified variable');
