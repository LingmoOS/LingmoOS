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
test_needs_command('shellcheck');
test_needs_srcdir_switch();

my @todofiles = qw(
    dselect/methods/disk/install.sh
    dselect/methods/disk/setup.sh
    dselect/methods/disk/update.sh
    dselect/methods/media/install.sh
    dselect/methods/media/setup.sh
    dselect/methods/media/update.sh
);
my @files = qw(
    autogen
    build-aux/gen-release
    build-aux/get-vcs-id
    build-aux/get-version
    build-aux/run-script
    debian/dpkg.cron.daily
    debian/dpkg.postrm
    src/dpkg-db-backup.sh
    src/dpkg-db-keeper.sh
    src/dpkg-maintscript-helper.sh
);
my @shellcheck_opts = (
    '--external-sources', # Allow checking external source files.
    '--exclude=SC1090', # Allow non-constant source.
    '--exclude=SC2039', # Allow local keyword.
    '--exclude=SC2166', # Allow -a and -o.
    '--exclude=SC2034', # Allow unused variables for colors.
    '--exclude=SC3043', # Allow local keyword.
);

plan tests => scalar @files;

sub shell_syntax_ok
{
    my $file = shift;

    my $tags = qx(shellcheck @shellcheck_opts $file 2>&1);

    # Fixup the output:
    chomp $tags;

    my $ok = length $tags == 0;

    ok($ok, 'shellcheck');
    if (not $ok) {
        diag($tags);
    }
}

foreach my $file (@files) {
    shell_syntax_ok($file);
}
