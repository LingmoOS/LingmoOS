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

use Test::More tests => 9;
use Test::Dpkg qw(:paths);

BEGIN {
    use_ok('Dpkg::Conf');
}

my $datadir = test_get_data_path();

my ($conf, $count, @opts);

my @expected_long_opts = (
'--option-double-quotes=value double quotes',
'--option-single-quotes=value single quotes',
'--option-space=value words space',
qw(
--option-dupe=value1
--option-name=value-name
--option-indent=value-indent
--option-equal=value-equal=subvalue-equal
--option-noequal=value-noequal
--option-dupe=value2
--option-simple
--option-dash=value-dash
--option-dupe=value3
--l=v
));
my @expected_short_opts = qw(
-o=vd
-s
);

$conf = Dpkg::Conf->new();
local $SIG{__WARN__} = sub { };
$count = $conf->load("$datadir/config-mixed");
delete $SIG{__WARN__};
is($count, 13, 'Load a config file, only long options');

@opts = $conf->get_options();
is_deeply(\@opts, \@expected_long_opts, 'Parse long options');

$conf = Dpkg::Conf->new(allow_short => 1);
$count = $conf->load("$datadir/config-mixed");
is($count, 15, 'Load a config file, mixed options');

@opts = $conf->get_options();
my @expected_mixed_opts = ( @expected_long_opts, @expected_short_opts );
is_deeply(\@opts, \@expected_mixed_opts, 'Parse mixed options');

my $expected_mixed_output = <<'MIXED';
option-double-quotes = "value double quotes"
option-single-quotes = "value single quotes"
option-space = "value words space"
option-dupe = "value1"
option-name = "value-name"
option-indent = "value-indent"
option-equal = "value-equal=subvalue-equal"
option-noequal = "value-noequal"
option-dupe = "value2"
option-simple
option-dash = "value-dash"
option-dupe = "value3"
l = "v"
-o = "vd"
-s
MIXED

is($conf->output, $expected_mixed_output, 'Output mixed options');

my $expected_filter;

$expected_filter = <<'FILTER';
l = "v"
-o = "vd"
-s
FILTER

$conf = Dpkg::Conf->new(allow_short => 1);
$conf->load("$datadir/config-mixed");
$conf->filter(remove => sub { $_[0] =~ m/^--option/ });
is($conf->output, $expected_filter, 'Filter remove');

$expected_filter = <<'FILTER';
option-double-quotes = "value double quotes"
option-single-quotes = "value single quotes"
FILTER

$conf = Dpkg::Conf->new(allow_short => 1);
$conf->load("$datadir/config-mixed");
$conf->filter(keep => sub { $_[0] =~ m/^--option-[a-z]+-quotes/ });
is($conf->output, $expected_filter, 'Filter keep');

$expected_filter = <<'FILTER';
l = "v"
FILTER

$conf = Dpkg::Conf->new(allow_short => 1);
$conf->load("$datadir/config-mixed");
$conf->filter(remove => sub { $_[0] =~ m/^--option/ },
              keep => sub { $_[0] =~ m/^--/ });
is($conf->output, $expected_filter, 'Filter keep and remove');
