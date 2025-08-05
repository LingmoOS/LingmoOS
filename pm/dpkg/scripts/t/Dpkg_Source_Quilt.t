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

use Test::More tests => 2;
use Test::Dpkg qw(:paths);

BEGIN {
    use_ok('Dpkg::Source::Quilt');
}

my $datadir = test_get_data_path();

my $quilt;
my (@series_got, @series_exp);

$quilt = Dpkg::Source::Quilt->new("$datadir/parse");
@series_got = $quilt->series();
@series_exp = qw(change-a.patch change-b.patch change-c.patch change-d.patch);
is_deeply(\@series_got, \@series_exp, 'Parsed series file matches ref');

# TODO: Add actual test cases.
