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

use Test::More tests => 4;

use File::Spec;
use File::Compare;
use File::Temp;

use Dpkg::IPC;

my $srcdir = $ENV{srcdir} || '.';
my $datadir = "$srcdir/t/dpkg_mergechangelogs";

sub test_merge {
    my ($expected_file, @options) = @_;
    my $fh = File::Temp->new();
    spawn(exec => [ $ENV{PERL}, "$srcdir/dpkg-mergechangelogs.pl", @options ],
	  to_handle => $fh, error_to_file => '/dev/null',
	  wait_child => 1, nocheck => 1);
    my $res = compare($expected_file, $fh->filename);
    if ($res) {
	system('diff', '-u', $expected_file, $fh->filename);
    }
    ok($res == 0, "merged changelog matches expected one ($expected_file)");
}

my $has_alg_merge = 1;
eval 'use Algorithm::Merge;';
if ($@) {
    $has_alg_merge = 0;
}

my @input = ("$datadir/ch-old", "$datadir/ch-a", "$datadir/ch-b");
if ($has_alg_merge) {
    test_merge("$datadir/ch-merged", @input);
    test_merge("$datadir/ch-merged-pr", '-m', @input);
    test_merge("$datadir/ch-unreleased-merged", '--merge-unreleased',
        ("$datadir/ch-unreleased-old",
         "$datadir/ch-unreleased-a",
         "$datadir/ch-unreleased-b"));
} else {
    test_merge("$datadir/ch-merged-basic", @input);
    test_merge("$datadir/ch-merged-pr-basic", '-m', @input);
    test_merge("$datadir/ch-unreleased-merged-basic", '--merge-unreleased',
        ("$datadir/ch-unreleased-old",
         "$datadir/ch-unreleased-a",
         "$datadir/ch-unreleased-b"));
}
test_merge("$datadir/ch-badver-merged",  ("$datadir/ch-badver-old",
    "$datadir/ch-badver-a", "$datadir/ch-badver-b"));
