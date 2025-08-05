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

use IPC::Cmd qw(can_run);

use Dpkg::ErrorHandling;
use Dpkg::IPC;
use Dpkg::Version;

report_options(quiet_warnings => 1);

my @tests = <DATA>;
my @ops = qw(
    < << lt
    <= le
    = eq
    >= ge
    > >> gt
);

plan tests => scalar(@tests) * (3 * scalar(@ops) + 4) + 27;

my $have_dpkg = can_run('dpkg');

sub dpkg_vercmp {
     my ($a, $cmp, $b) = @_;
     my $stderr;

     spawn(exec => [ 'dpkg', '--compare-versions', '--', $a, $cmp, $b ],
           error_to_string => \$stderr, wait_child => 1, nocheck => 1);
     diag("dpkg --compare-versions error=$?: $stderr") if $? and $? != 256;

     return $? == 0;
}

sub obj_vercmp {
     my ($a, $cmp, $b) = @_;
     return $a < $b  if $cmp eq '<<';
     return $a lt $b if $cmp eq 'lt';
     return $a <= $b if $cmp eq '<=' or $cmp eq '<';
     return $a le $b if $cmp eq 'le';
     return $a == $b if $cmp eq '=';
     return $a eq $b if $cmp eq 'eq';
     return $a >= $b if $cmp eq '>=' or $cmp eq '>';
     return $a ge $b if $cmp eq 'ge';
     return $a > $b  if $cmp eq '>>';
     return $a gt $b if $cmp eq 'gt';
}

my $truth = {
    '-1' => {
	'<<' => 1, 'lt' => 1,
	'<=' => 1, 'le' => 1, '<' => 1,
	'=' => 0, 'eq' => 0,
	'>=' => 0, 'ge' => 0, '>' => 0,
	'>>' => 0, 'gt' => 0,
    },
    '0' => {
	'<<' => 0, 'lt' => 0,
	'<=' => 1, 'le' => 1, '<' => 1,
	'=' => 1, 'eq' => 1,
	'>=' => 1, 'ge' => 1, '>' => 1,
	'>>' => 0, 'gt' => 0,
    },
    '1' => {
	'<<' => 0, 'lt' => 0,
	'<=' => 0, 'le' => 0, '<' => 0,
	'=' => 0, 'eq' => 0,
	'>=' => 1, 'ge' => 1, '>' => 1,
	'>>' => 1, 'gt' => 1,
    },
};

# Handling of empty/invalid versions
my $empty = Dpkg::Version->new('');
ok($empty eq '', "Dpkg::Version->new('') eq ''");
ok($empty->as_string() eq '', "Dpkg::Version->new('')->as_string() eq ''");
ok(!$empty->is_valid(), 'empty version is invalid');
$empty = Dpkg::Version->new('-0');
ok($empty eq '', "Dpkg::Version->new('-0') eq '-0'");
ok($empty->as_string() eq '-0', "Dpkg::Version->new('-0')->as_string() eq '-0'");
ok(!$empty->is_valid(), 'empty upstream version is invalid');
$empty = Dpkg::Version->new('0:-0');
ok($empty eq '0:-0', "Dpkg::Version->new('0:-0') eq '0:-0'");
ok($empty->as_string() eq '0:-0', "Dpkg::Version->new('0:-0')->as_string() eq '0:-0'");
ok(!$empty->is_valid(), 'empty upstream version with epoch is invalid');
$empty = Dpkg::Version->new(':1.0');
ok($empty eq ':1.0', "Dpkg::Version->new(':1.0') eq ':1.0'");
ok($empty->as_string() eq ':1.0', "Dpkg::Version->new(':1.0')->as_string() eq ':1.0'");
ok(!$empty->is_valid(), 'empty epoch is invalid');
$empty = Dpkg::Version->new('1.0-');
ok($empty eq '1.0-', "Dpkg::Version->new('1.0-') eq '1.0-'");
ok($empty->as_string() eq '1.0-', "Dpkg::Version->new('1.0-')->as_string() eq '1.0-'");
ok(!$empty->is_valid(), 'empty revision is invalid');
my $ver = Dpkg::Version->new('10a:5.2');
ok(!$ver->is_valid(), 'bad epoch is invalid');
ok(!$ver, 'bool eval of invalid leads to false');
ok($ver eq '10a:5.2', 'invalid still same string 1/2');
$ver = Dpkg::Version->new('5.2@3-2');
ok($ver eq '5.2@3-2', 'invalid still same string 2/2');
ok(!$ver->is_valid(), 'illegal character is invalid');
$ver = Dpkg::Version->new('foo5.2');
ok(!$ver->is_valid(), 'version does not start with digit 1/2');
$ver = Dpkg::Version->new('0:foo5.2');
ok(!$ver->is_valid(), 'version does not start with digit 2/2');

# Native and non-native versions
$ver = Dpkg::Version->new('1.0');
ok($ver->is_native(), 'upstream version is native');
$ver = Dpkg::Version->new('1:1.0');
ok($ver->is_native(), 'upstream version w/ epoch is native');
$ver = Dpkg::Version->new('1:1.0:1.0');
ok($ver->is_native(), 'upstream version w/ epoch and colon is native');
$ver = Dpkg::Version->new('1.0-1');
ok(!$ver->is_native(), 'upstream version w/ revision is not native');
$ver = Dpkg::Version->new('1.0-1.0-1');
ok(!$ver->is_native(), 'upstream version w/ dash and revision is not native');

# Comparisons
foreach my $case (@tests) {
    my ($a, $b, $res) = split ' ', $case;
    my $va = Dpkg::Version->new($a, check => 1);
    my $vb = Dpkg::Version->new($b, check => 1);

    is("$va", $a, "String representation of Dpkg::Version($a) is $a");
    is("$vb", $b, "String representation of Dpkg::Version($b) is $b");

    is(version_compare($a, $b), $res, "$a cmp $b => $res");
    is($va <=> $vb, $res, "Dpkg::Version($a) <=> Dpkg::Version($b) => $res");
    foreach my $op (@ops) {
        my $norm_op = version_normalize_relation($op);
	if ($truth->{$res}{$op}) {
	    ok(version_compare_relation($a, $norm_op, $b), "$a $op $b => true");
	    ok(obj_vercmp($va, $op, $vb), "Dpkg::Version($a) $op Dpkg::Version($b) => true");

            SKIP: {
                skip 'dpkg not available', 1 if not $have_dpkg;

                ok(dpkg_vercmp($a, $op, $b), "dpkg --compare-versions -- $a $op $b => true");
            }
	} else {
	    ok(!version_compare_relation($a, $norm_op, $b), "$a $op $b => false");
	    ok(!obj_vercmp($va, $op, $vb), "Dpkg::Version($a) $op Dpkg::Version($b) => false");

            SKIP: {
                skip 'dpkg not available', 1 if not $have_dpkg;

                ok(!dpkg_vercmp($a, $op, $b), "dpkg --compare-versions -- $a $op $b => false");
            }
	}
    }
}

__DATA__
1.0-1 2.0-2 -1
2.2~rc-4 2.2-1 -1
2.2-1 2.2~rc-4 1
1.0000-1 1.0-1 0
1 0:1 0
0 0:0-0 0
2:2.5 1:7.5 1
1:0foo 0foo 1
0:0foo 0foo 0
0foo 0foo 0
0foo-0 0foo 0
0foo 0foo-0 0
0foo 0fo 1
0foo-0 0foo+ -1
0foo~1 0foo -1
0foo~foo+Bar 0foo~foo+bar -1
0foo~~ 0foo~ -1
1~ 1 -1
12345+that-really-is-some-ver-0 12345+that-really-is-some-ver-10 -1
0foo-0 0foo-01 -1
0foo.bar 0foobar 1
0foo.bar 0foo1bar 1
0foo.bar 0foo0bar 1
0foo1bar-1 0foobar-1 -1
0foo2.0 0foo2 1
0foo2.0.0 0foo2.10.0 -1
0foo2.0 0foo2.0.0 -1
0foo2.0 0foo2.10 -1
0foo2.1 0foo2.10 -1
1.09 1.9 0
1.0.8+nmu1 1.0.8 1
3.11 3.10+nmu1 1
0.9j-20080306-4 0.9i-20070324-2 1
1.2.0~b7-1 1.2.0~b6-1 1
1.011-1 1.06-2 1
0.0.9+dfsg1-1 0.0.8+dfsg1-3 1
4.6.99+svn6582-1 4.6.99+svn6496-1 1
53 52 1
0.9.9~pre122-1 0.9.9~pre111-1 1
2:2.3.2-2+lenny2 2:2.3.2-2 1
1:3.8.1-1 3.8.GA-1 1
1.0.1+gpl-1 1.0.1-2 1
1a 1000a -1
