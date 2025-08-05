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
test_needs_command('i18nspector');
test_needs_srcdir_switch();

my @files = Test::Dpkg::all_po_files();

plan skip_all => 'no PO files distributed' if @files == 0;
plan tests => scalar @files;

sub po_ok {
    my $file = shift;

    my $tags = qx(i18nspector \"$file\" 2>&1);

    # Fixup the output:
    $tags =~ s/^.*\.pot: boilerplate-in-initial-comments .*\n//mg;
    $tags =~ s/^.*\.po: duplicate-header-field X-POFile-SpellExtra\n//mg;
    chomp $tags;

    my $ok = length $tags == 0;

    ok($ok, "PO check $file");
    if (not $ok) {
        diag($tags);
    }
}

for my $file (@files) {
    po_ok($file);
}
