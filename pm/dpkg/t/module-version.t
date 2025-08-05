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
use Module::Metadata;

test_needs_srcdir_switch();

my @files = Test::Dpkg::all_perl_modules();

plan tests => scalar @files;

sub module_version_ok
{
    my $file = shift;

    my $mod = Module::Metadata->new_from_file($file, collect_pod => 1);
    my $modver = $mod->version();
    my $podver;

    SKIP: {
        if ($mod->contains_pod()) {
            my $in_changes = 0;

            foreach my $sect ($mod->pod_inside) {
                if ($sect eq 'CHANGES') {
                    $in_changes = 1;
                    next;
                }

                if ($in_changes and $sect =~ m/^Version ([0-9x.]+)/) {
                    $podver = $1;
                    last;
                }
            }

            if (not $in_changes) {
                fail("module $file does not contain a CHANGES POD section");
                return;
            }
        } else {
            skip("module $file does not contain POD", 1);
        }

        if (defined $podver and $podver eq '0.xx') {
            ok($modver =~ m/^0.\d\d$/,
               "module $file version $modver is POD version 0.xx");
        } else {
            ok($modver eq $podver,
               "module $file version $modver == POD version $podver");
        }
    }
}

foreach my $file (@files) {
    module_version_ok($file);
};
