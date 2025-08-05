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
    use_ok('Dpkg::Source::Format');
}

my $format = Dpkg::Source::Format->new();
my @format_parts;
my $format_string;

$format->set('4.3 (variant)');
@format_parts = $format->get();
is_deeply(\@format_parts, [ qw(4 3 variant) ], 'decomposition of format');
$format_string = $format->get();
ok($format_string eq '4.3 (variant)', 'function stringification of format');
$format_string = "$format";
ok($format_string eq '4.3 (variant)', 'operator stringification of format');

$format->set('5.5');
$format_string = $format->get();
ok($format_string eq '5.5', 'missing variant');

$format->set('6');
$format_string = $format->get();
ok($format_string eq '6.0', 'implied minor');

my %format_bogus = (
    'a' => 'require numerical major',
    '7.a' => 'require numerical minor',
    '.5' => 'require non-empty major',
    '7.' => 'require non-empty minor',
    '7.0 ()' => 'require non-empty variant',
    '7.0 (  )' => 'require non-space variant',
    '7.0 (VARIANT)' => 'require lower-case variant',
    '7.6.5' => 'excess version part',
);

foreach my $format_bogus (sort keys %format_bogus) {
    eval {
        $format->set($format_bogus);
    };
    ok($@, $format_bogus{$format_bogus});
};

# TODO: Add actual test cases.
