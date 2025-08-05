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

use Test::More tests => 5;
use Test::Dpkg qw(:paths);

BEGIN {
    use_ok('Dpkg::Control::Tests');
}

my $datadir = test_get_data_path();

sub parse_tests {
    my $path = shift;

    my $tests = Dpkg::Control::Tests->new();
    eval {
        $tests->load($path);
        1;
    } or return;

    return $tests;
}

my $tests;

$tests = parse_tests("$datadir/tests-missing-fields");
is($tests, undef, 'autopkgtest missing required fields');

$tests = parse_tests("$datadir/tests-plain-text");
is($tests, undef, 'autopkgtest is not in deb822 format');

my $expected = <<'TESTS';
Tests: aaa, bbb, ccc

Tests: danger, warning
Restrictions: rw-build-tree, needs-root, breaks-testbed

Tests: depends
Depends: @, @builddeps@, extra-package

Tests: dir
Tests-Directory: .

Tests: feature

Tests: class
Classes: self-test

Test-Command: command arg1 arg2

TESTS

$tests = parse_tests("$datadir/tests-valid");
ok(defined $tests, 'Valid autopkgtest control file');
is($tests->output(), $expected, 'autopkgtest control file dumped');
