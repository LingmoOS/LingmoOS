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

use Test::More tests => 39;

BEGIN {
    use_ok('Dpkg::BuildTypes');
}

ok(build_is(BUILD_DEFAULT | BUILD_FULL), 'build is default full');
is(get_build_options_from_type(), 'full', 'build is full');

set_build_type(BUILD_DEFAULT | BUILD_BINARY, '--default-binary');
is(get_build_options_from_type(), 'binary', 'build is binary');
ok(build_is(BUILD_DEFAULT | BUILD_BINARY), 'build is default binary');

set_build_type(BUILD_SOURCE | BUILD_ARCH_INDEP, '--build=source,all');
is(get_build_options_from_type(), 'source,all', 'build is source,all');

set_build_type_from_options('any,all', '--build=any,all', nocheck => 1);
is(get_build_options_from_type(), 'binary', 'build is binary from any,all');
ok(build_is(BUILD_BINARY), 'build is any,all');

set_build_type_from_options('binary', '--build=binary', nocheck => 1);
is(get_build_options_from_type(), 'binary', 'build is binary');
ok(build_is(BUILD_BINARY), 'build is binary');

set_build_type_from_options('source,all', '--build=source,all', nocheck => 1);
ok(build_is(BUILD_SOURCE | BUILD_ARCH_INDEP), 'build source,all is source,all');
ok(!build_is(BUILD_SOURCE | BUILD_ARCH_DEP), 'build source,all is not source,any');
ok(build_has_any(BUILD_SOURCE), 'build source,all has_any source');
ok(build_has_any(BUILD_ARCH_INDEP), 'build source,all has_any any');
ok(build_has_none(BUILD_DEFAULT), 'build source,all has_none default');
ok(build_has_none(BUILD_ARCH_DEP), 'build source,all has_none any');
ok(!build_has_all(BUILD_BINARY), 'build source,all not has_all binary');
ok(!build_has_all(BUILD_SOURCE | BUILD_ARCH_DEP),
   'build source,all not has_all source,any');
ok(!build_has_all(BUILD_FULL), 'build source,all has_all full');

set_build_type_from_targets('build-arch,build-indep',
                            '--targets=build-arch,build-indep', nocheck => 1);
is(get_build_options_from_type(), 'binary',
                                  'build is binary from build-arch,build-indep');
ok(build_is(BUILD_BINARY), 'build is binary from build-arch,build-indep');

set_build_type_from_targets('binary', '--targets=binary', nocheck => 1);
is(get_build_options_from_type(), 'binary', 'build is binary from binary');
ok(build_is(BUILD_BINARY), 'build is binary from binary');

set_build_type_from_targets('clean,binary-indep',
                            '--targets=clean,binary-indep', nocheck => 1);
ok(build_is(BUILD_SOURCE | BUILD_ARCH_INDEP), 'build source,all is source,all');
ok(!build_is(BUILD_SOURCE | BUILD_ARCH_DEP), 'build source,all is not source,any');
ok(build_has_any(BUILD_SOURCE), 'build source,all has_any source');
ok(build_has_any(BUILD_ARCH_INDEP), 'build source,all has_any any');
ok(build_has_none(BUILD_DEFAULT), 'build source,all has_none default');
ok(build_has_none(BUILD_ARCH_DEP), 'build source,all has_none any');
ok(!build_has_all(BUILD_BINARY), 'build source,all not has_all binary');
ok(!build_has_all(BUILD_SOURCE | BUILD_ARCH_DEP),
   'build source,all not has_all source,any');
ok(!build_has_all(BUILD_FULL), 'build source,all has_all full');

set_build_type(BUILD_BINARY, '--build=binary', nocheck => 1);
ok(build_is(BUILD_BINARY), 'build binary is binary');
ok(build_has_any(BUILD_ARCH_DEP), 'build binary has_any any');
ok(build_has_any(BUILD_ARCH_INDEP), 'build binary has_any all');
ok(build_has_all(BUILD_BINARY), 'build binary has_all binary');
ok(build_has_none(BUILD_SOURCE), 'build binary has_none source');

set_build_type(BUILD_FULL, '--build=full', nocheck => 1);
ok(build_has_any(BUILD_SOURCE), 'build full has_any source');
ok(build_has_all(BUILD_BINARY), 'build full has_all binary');
