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

BEGIN {
    use_ok('Dpkg::Getopt');
}

my @expect_argv;

@ARGV = normalize_options(args => [ qw(-a -bfoo -c var) ]);
@expect_argv = qw(-a -b foo -c var);
is_deeply(\@ARGV, \@expect_argv, 'unbundle short options');

@ARGV = normalize_options(args => [ qw(--option-a --option-b value --option-c=value) ]);
@expect_argv = qw(--option-a --option-b value --option-c value);
is_deeply(\@ARGV, \@expect_argv, 'unbundle long options');

@ARGV = normalize_options(args => [ qw(-aaa -bbb --option-a=oa -- --opt=arg -dval) ],
                          delim => '--');
@expect_argv = qw(-a aa -b bb --option-a oa -- --opt=arg -dval);
is_deeply(\@ARGV, \@expect_argv, 'unbundle options with delimiter');
