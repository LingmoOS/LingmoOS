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
use Test::Dpkg qw(:needs);

test_needs_author();
test_needs_command('cppcheck');
test_needs_srcdir_switch();

plan tests => 1;

my $builddir = $ENV{abs_top_builddir} || '.';

my @cppcheck_opts = (qw(
  --quiet
  --force
  --error-exitcode=2
  --inline-suppr
  --check-level=exhaustive
  --suppressions-list=t/cppcheck/cppcheck.supp
  --std=c99 --std=c++14
  -Ilib
  -Ilib/compat
  -Isrc/common
),
  "-I$builddir",
qw(
  -D__GNUC__=12
  -D__GNUC_MINOR__=0
  -D_DIRENT_HAVE_D_TYPE=1
  -DWITH_LIBSELINUX=1
  -DLIBDPKG_VOLATILE_API=1
), (
  '--enable=warning,performance,portability,style',
  '--template=\'{file}:{line}: {severity} ({id}): {message}\''
));
my $tags = qx(cppcheck @cppcheck_opts . 2>&1);

# Fixup the output:
chomp $tags;

my $ok = length $tags == 0;

ok($ok, 'cppcheck');
if (not $ok) {
    diag($tags);
}
