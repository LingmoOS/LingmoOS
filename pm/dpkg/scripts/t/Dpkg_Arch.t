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

use Test::More tests => 7142;

use_ok('Dpkg::Arch', qw(debarch_to_debtuple debarch_to_multiarch
                        debarch_eq debarch_is debarch_is_wildcard
                        debarch_is_illegal
                        debarch_to_abiattrs debarch_to_cpubits
                        debarch_list_parse
                        debtuple_to_debarch gnutriplet_to_debarch
                        debtuple_to_gnutriplet gnutriplet_to_debtuple
                        get_host_gnu_type
                        get_valid_arches));

my $KNOWN_ARCHES_TOTAL = 206;
my @valid_arches = get_valid_arches();

sub get_valid_wildcards
{
    my %wildcards;
    my @wildcards_base = qw(
        any
        any-any
        any-any-any
        any-any-any-any
    );

    foreach my $archname (@valid_arches) {
        my @tuple = debarch_to_debtuple($archname);

        my @wildcards_arch = (
            # Two element tuples.
            "$tuple[2]-any",
            "any-$tuple[3]",

            # Three element tuples.
            "$tuple[1]-$tuple[2]-any",
            "$tuple[1]-any-$tuple[3]",
            "$tuple[1]-any-any",
            "any-$tuple[2]-$tuple[3]",
            "any-$tuple[2]-any",
            "any-any-$tuple[3]",

            # Four element tuples.
            "$tuple[0]-$tuple[1]-$tuple[2]-any",
            "$tuple[0]-$tuple[1]-any-$tuple[3]",
            "$tuple[0]-$tuple[1]-any-any",
            "$tuple[0]-any-$tuple[2]-$tuple[3]",
            "$tuple[0]-any-$tuple[2]-any",
            "$tuple[0]-any-any-$tuple[3]",
            "$tuple[0]-any-any-any",
            "any-$tuple[1]-$tuple[2]-$tuple[3]",
            "any-$tuple[1]-$tuple[2]-any",
            "any-$tuple[1]-any-$tuple[3]",
            "any-$tuple[1]-any-any",
            "any-any-$tuple[2]-$tuple[3]",
            "any-any-$tuple[2]-any",
            "any-any-any-$tuple[3]",
        );

        foreach my $wildcard ((@wildcards_base, @wildcards_arch)) {
            push @{$wildcards{$wildcard}}, $archname;
        }
    }

    return \%wildcards;
}

my @tuple_new;
my @tuple_ref;

@tuple_new = debarch_to_debtuple('unknown');
is_deeply(\@tuple_new, [], 'unknown tuple');

@tuple_ref = qw(base gnu linux amd64);
@tuple_new = debarch_to_debtuple('amd64');
is_deeply(\@tuple_new, \@tuple_ref, 'valid tuple');

@tuple_ref = qw(base gnu linux amd64);
@tuple_new = debarch_to_debtuple('amd64');
is_deeply(\@tuple_new, \@tuple_ref, 'valid tuple');
@tuple_new = debarch_to_debtuple('linux-amd64');
is_deeply(\@tuple_new, \@tuple_ref, 'valid tuple');

is(debarch_to_multiarch('i386'), 'i386-linux-gnu',
   'normalized i386 multiarch triplet');
is(debarch_to_multiarch('amd64'), 'x86_64-linux-gnu',
   'normalized amd64 multiarch triplet');

ok(!debarch_eq('amd64', 'i386'), 'no match, simple arch');
ok(!debarch_eq('', 'amd64'), 'no match, empty first arch');
ok(!debarch_eq('amd64', ''), 'no match, empty second arch');
ok(!debarch_eq('amd64', 'unknown'), 'no match, with first unknown arch');
ok(!debarch_eq('unknown', 'i386'), 'no match, second unknown arch');
ok(debarch_eq('unknown', 'unknown'), 'match equal unknown arch');
ok(debarch_eq('amd64', 'amd64'), 'match equal known arch');
ok(debarch_eq('amd64', 'linux-amd64'), 'match implicit linux arch');

ok(!debarch_is('unknown', 'linux-any'), 'no match unknown on wildcard cpu');
ok(!debarch_is('unknown', 'any-amd64'), 'no match unknown on wildcard os');
ok(!debarch_is('amd64', 'unknown'), 'no match amd64 on unknown wildcard');
ok(!debarch_is('amd64', 'unknown-any'), 'no match amd64 on unknown wildcard');
ok(!debarch_is('amd64', 'any-unknown'), 'no match amd64 on unknown wildcard');
ok(debarch_is('unknown', 'any'), 'match unknown on global wildcard');
ok(debarch_is('linux-amd64', 'linux-any'), 'match implicit linux-amd64 on wildcard cpu');
ok(debarch_is('linux-amd64', 'any-amd64'), 'match implicit linux-amd64 on wildcard os');

my $wildcards = get_valid_wildcards();

foreach my $wildcard (sort keys %{$wildcards}) {
    ok(debarch_is_wildcard($wildcard), "$wildcard is a wildcard");

    foreach my $arch (sort @{$wildcards->{$wildcard}}) {
        ok(debarch_is($arch, $wildcard), "wildcard $wildcard matches $arch");
    }
}

ok(!debarch_is_wildcard('unknown'), 'unknown is not a wildcard');
ok(!debarch_is_wildcard('all'), 'all is not a wildcard');
ok(!debarch_is_wildcard('amd64'), '<arch> is not a wildcard');

ok(!debarch_is_illegal('0'), '');
ok(!debarch_is_illegal('a'), '');
ok(!debarch_is_illegal('amd64'), '');
ok(!debarch_is_illegal('!arm64'), '');
ok(!debarch_is_illegal('kfreebsd-any'), '');
ok(debarch_is_illegal('!amd64!arm'), '');
ok(debarch_is_illegal('arch%name'), '');
ok(debarch_is_illegal('-any'), '');
ok(debarch_is_illegal('!'), '');

my @arch_new;
my @arch_ref;

@arch_ref = qw(amd64 !arm64 linux-i386 !kfreebsd-any);
@arch_new = debarch_list_parse('amd64  !arm64   linux-i386 !kfreebsd-any');
is_deeply(\@arch_new, \@arch_ref, 'parse valid arch list');

@arch_ref = qw(amd64 arm64 linux-i386 kfreebsd-any);
@arch_new = debarch_list_parse('amd64  arm64   linux-i386 kfreebsd-any', positive => 1);
is_deeply(\@arch_new, \@arch_ref, 'parse valid positive arch list');

eval { @arch_new = debarch_list_parse('!amd64!arm64') };
ok($@, 'parse concatenated arches failed');

eval { @arch_new = debarch_list_parse('amd64 !arm64 !mips', positive => 1) };
ok($@, 'parse disallowed negated arches failed');

is(debarch_to_abiattrs(undef), undef, 'undef ABI attrs');
is_deeply([ debarch_to_abiattrs('amd64') ], [ qw(64 little) ], 'amd64 ABI attrs');
is_deeply([ debarch_to_abiattrs('x32') ], [ qw(32 little) ], 'x32 ABI attrs');

is(debarch_to_cpubits(undef), undef, 'undef CPU bits');
is(debarch_to_cpubits('i386'), 32, 'i386 CPU bits');
is(debarch_to_cpubits('amd64'), 64, 'amd64 CPU bits');

is(debtuple_to_debarch((undef) x 4), undef, 'undef debtuple');
is(debtuple_to_debarch('base', 'gnu', 'linux', 'amd64'), 'amd64', 'known debtuple');
is(debtuple_to_debarch('unknown', 'unknown', 'unknown', 'unknown'), undef, 'unknown debtuple');

is(gnutriplet_to_debarch(undef), undef, 'undef gnutriplet');
is(gnutriplet_to_debarch('unknown-unknown-unknown'), undef, 'unknown gnutriplet');
is(gnutriplet_to_debarch('x86_64-linux-gnu'), 'amd64', 'known gnutriplet');

foreach my $arch (@valid_arches) {
    my @tuple = debarch_to_debtuple($arch);
    is(debtuple_to_debarch(@tuple), $arch,
       "bijective arch $arch to tuple @tuple");

    my $triplet = debtuple_to_gnutriplet(@tuple);
    is_deeply([ gnutriplet_to_debtuple($triplet) ], \@tuple,
              "bijective triplet $triplet to tuple @tuple");
}

is(scalar @valid_arches, $KNOWN_ARCHES_TOTAL,
    'expected amount of known architectures');

{
    local $ENV{CC} = 'false';
    is(get_host_gnu_type(), '', 'CC does not support -dumpmachine');

    $ENV{CC} = 'echo CC';
    is(get_host_gnu_type(), 'CC -dumpmachine',
       'CC does not report expected synthetic result for -dumpmachine');
}
