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

use Test::More tests => 60;
use Test::Dpkg qw(:paths);

use Dpkg ();
use Dpkg::Arch qw(get_host_arch);

$ENV{DEB_BUILD_ARCH} = 'amd64';
$ENV{DEB_HOST_ARCH} = 'amd64';

use_ok('Dpkg::Substvars');

my $datadir = test_get_data_path();

my $output;
my $expected;

my $s = Dpkg::Substvars->new();

$s->load("$datadir/substvars1");

# simple value tests
is($s->get('var1'), 'Some value', 'var1');
is($s->get('var2'), 'Some other value', 'var2');
is($s->get('var3'), 'Yet another value', 'var3');
is($s->get('var4'), undef, 'no var4');
is($s->get('optional-var5'), 'Optionally used value', 'optional-var5');

# Set automatic variable
$s->set_as_auto('var_auto', 'auto');
is($s->get('var_auto'), 'auto', 'get var_auto');

$expected = <<'VARS';
optional-var5?=Optionally used value
var1=Some value
var2=Some other value
var3=Yet another value
VARS
is($s->output(), $expected, 'No automatic variables output');

# overriding
$s->set('var1', 'New value');
is($s->get('var1'), 'New value', 'var1 updated');

# deleting
$s->delete('var3');
is($s->get('var3'), undef, 'var3 deleted');

# default variables
is($s->get('Newline'), "\n", 'newline');
is($s->get('Space'), ' ', 'space');
is($s->get('Tab'), "\t", 'tab');
is($s->get('dpkg:Version'), $Dpkg::PROGVERSION, 'dpkg version 1');

# special variables
is($s->get('Arch'), undef, 'no arch');
$s->set_arch_substvars();
is($s->get('Arch'), get_host_arch(), 'arch');

is($s->get('vendor:Id'), undef, 'no vendor id');
is($s->get('vendor:Name'), undef, 'no vendor name');
$s->set_vendor_substvars();
is($s->get('vendor:Id'), 'debian', 'vendor id');
is($s->get('vendor:Name'), 'Debian', 'vendor name');

is($s->get($_), undef, 'no ' . $_) for qw/binary:Version source:Version source:Upstream-Version/;
$s->set_version_substvars('1:2.3.4~5-6.7.8~nmu9', '1:2.3.4~5-6.7.8~nmu9+bin0');
is($s->get('binary:Version'), '1:2.3.4~5-6.7.8~nmu9+bin0', 'binary:Version');
is($s->get('source:Version'), '1:2.3.4~5-6.7.8~nmu9', 'source:Version');
is($s->get('source:Upstream-Version'), '1:2.3.4~5', 'source:Upstream-Version');
$s->set_version_substvars('2.3.4~5-6.7.8~nmu9+b1', '1:2.3.4~5-6.7.8~nmu9+b1');
is($s->get('binary:Version'), '1:2.3.4~5-6.7.8~nmu9+b1', 'binary:Version');
is($s->get('source:Version'), '2.3.4~5-6.7.8~nmu9', 'source:Version');
is($s->get('source:Upstream-Version'), '2.3.4~5', 'source:Upstream-Version');
$s->set_version_substvars('1:2.3.4~5-6.7.8~nmu9+b0');
is($s->get('binary:Version'), '1:2.3.4~5-6.7.8~nmu9+b0', 'binary:Version');
is($s->get('source:Version'), '1:2.3.4~5-6.7.8~nmu9', 'source:Version');
is($s->get('source:Upstream-Version'), '1:2.3.4~5', 'source:Upstream-Version');

is($s->get($_), undef, 'no ' . $_) foreach qw(source:Synopsis source:Extended-Description);
$s->set_desc_substvars("short synopsis\nthis is the long\nextended text\n");
is($s->get('source:Synopsis'), 'short synopsis', 'contents of source:Synopsis');
is($s->get('source:Extended-Description'), "this is the long\nextended text\n",
    'contents of source:Extended-Description');

my %ctrl_fields = (
    'Some-Field' => 'some-value',
    'Other-Field' => 'other-value',
    'Alter-Field' => 'alter-value',
);
is($s->get($_), undef, 'no ' . $_) foreach sort keys %ctrl_fields;
$s->set_field_substvars(\%ctrl_fields, 'ctrl');
is($s->get('ctrl:Some-Field'), 'some-value', 'contents of ctrl:Some-Field');
is($s->get('ctrl:Other-Field'), 'other-value', 'contents of ctrl:Other-Field');
is($s->get('ctrl:Alter-Field'), 'alter-value', 'contents of ctrl:Alter-Field');

# Direct replace: few
is($s->substvars('This is a string ${var1} with variables ${binary:Version}'),
                 'This is a string New value with variables 1:2.3.4~5-6.7.8~nmu9+b0',
                 'direct replace, few times');

# Direct replace: many times (more than the recursive limit)
$s->set('dr', 'feed');
is($s->substvars('${dr}${dr}${dr}${dr}${dr}${dr}${dr}${dr}${dr}${dr}' .
                 '${dr}${dr}${dr}${dr}${dr}${dr}${dr}${dr}${dr}${dr}' .
                 '${dr}${dr}${dr}${dr}${dr}${dr}${dr}${dr}${dr}${dr}' .
                 '${dr}${dr}${dr}${dr}${dr}${dr}${dr}${dr}${dr}${dr}' .
                 '${dr}${dr}${dr}${dr}${dr}${dr}${dr}${dr}${dr}${dr}' .
                 '${dr}${dr}${dr}${dr}${dr}${dr}${dr}${dr}${dr}${dr}'),
    'feedfeedfeedfeedfeedfeedfeedfeedfeedfeed' .
    'feedfeedfeedfeedfeedfeedfeedfeedfeedfeed' .
    'feedfeedfeedfeedfeedfeedfeedfeedfeedfeed' .
    'feedfeedfeedfeedfeedfeedfeedfeedfeedfeed' .
    'feedfeedfeedfeedfeedfeedfeedfeedfeedfeed' .
    'feedfeedfeedfeedfeedfeedfeedfeedfeedfeed',
    'direct replace, many times');

# Add a test prefix to error and warning messages.
$s->set_msg_prefix('test ');

$output = q{};
$SIG{__WARN__} = sub { $output .= $_[0] };
is($s->substvars('This is a string with unknown variable ${blubb}'),
                 'This is a string with unknown variable ',
                 'substvars missing');
delete $SIG{__WARN__};
is($output,
   'Dpkg_Substvars.t: warning: test substitution variable ${blubb} used, but is not defined' . "\n",
   'missing variables warning');

# Recursive replace: Simple.
$s->set('rvar', 'recursive ${var1}');
is($s->substvars('This is a string with ${rvar}'),
                 'This is a string with recursive New value',
                 'recursive replace simple');

# Recursive replace: Constructed variables.
$s->set('partref', 'recursive result');
$s->set('part1', '${pa');
$s->set('part2', 'rtr');
$s->set('part3', 'ef}');
is($s->substvars('Constructed ${part1}${part2}${part3} replace'),
                 'Constructed recursive result replace',
                 'recursive constructed variable');

# Recursive replace: Cycle.
$s->set('ref0', '${ref1}');
$s->set('ref1', '${ref2}');
$s->set('ref2', '${ref0}');

eval {
    $s->substvars('Cycle reference ${ref0}');
    1;
};
$output = $@ // q{};
is($output,
   'Dpkg_Substvars.t: error: test too many ${ref0} substitutions ' .
   "(recursive?) in 'Cycle reference \${ref1}'\n",
   'recursive cyclic expansion is limited');

# Recursive replace: Billion laughs.
$s->set('ex0', ':)');
$s->set('ex1', '${ex0}${ex0}${ex0}${ex0}${ex0}${ex0}${ex0}${ex0}${ex0}${ex0}');
$s->set('ex2', '${ex1}${ex1}${ex1}${ex1}${ex1}${ex1}${ex1}${ex1}${ex1}${ex1}');
$s->set('ex3', '${ex2}${ex2}${ex2}${ex2}${ex2}${ex2}${ex2}${ex2}${ex2}${ex2}');
$s->set('ex4', '${ex3}${ex3}${ex3}${ex3}${ex3}${ex3}${ex3}${ex3}${ex3}${ex3}');
$s->set('ex5', '${ex4}${ex4}${ex4}${ex4}${ex4}${ex4}${ex4}${ex4}${ex4}${ex4}');
$s->set('ex6', '${ex5}${ex5}${ex5}${ex5}${ex5}${ex5}${ex5}${ex5}${ex5}${ex5}');
$s->set('ex7', '${ex6}${ex6}${ex6}${ex6}${ex6}${ex6}${ex6}${ex6}${ex6}${ex6}');
$s->set('ex8', '${ex7}${ex7}${ex7}${ex7}${ex7}${ex7}${ex7}${ex7}${ex7}${ex7}');
$s->set('ex9', '${ex8}${ex8}${ex8}${ex8}${ex8}${ex8}${ex8}${ex8}${ex8}${ex8}');

eval {
    $s->substvars('Billion laughs ${ex9}');
    1;
};
$output = $@ // q{};
is($output,
   'Dpkg_Substvars.t: error: test too many ${ex1} substitutions ' .
   "(recursive?) in 'Billion laughs :):):):):):):):):):):):):):)" .
   ':):):):):):):):):):):):):):):):):):):):):):):):):):):):):):)' .
   ':):):):):):):):):):):):):):):):):):):):):):):):):):):):):):)' .
   ':):):):):):):):):):):):):):):):):):):):):):):):):):):):):):)' .
   ':):):):):):):):):):):):):):):):):):):):):):):):):):):):):):)' .
   ':):):):):):):):):):):):):):):):):):):):):):):):):):):):):):)' .
   ':):):):):):):):):):):):):):):):):):):):):):):):):):):):):):)' .
   ':):):):):):):):):):):):):):):):):):):):):):):):):):):):):):)' .
   ':):):):):):):):):):):):):):):):):):):):):):):):):):):):):):)' .
   ':):):):):):):):):):):):):):):):):):):):):):):):):):):):):):)' .
   ':):):):):):):):):):):):):):):):):):):):):):):):):):):):):):)' .
   ':):):):):):):):):):):):):):):):):):):):):):):):):):):):):):)' .
   ':):):):):):):):):):):):):):):):):):):):):):):):):):):):):):)' .
   ':):):):):):):):):):):):):):):):):):):):):):):):):):):):):):)' .
   ':):):):):):):):):):):):):):):):):):):):):):):):):):):):):):)' .
   ':):):):):):):):):):):):):):):):):):):):):):):):):):):):):):)' .
   ':):):):):):):):):):):):):):):):):):):):):):):):):):)' .
   '${ex0}${ex0}${ex0}${ex0}${ex0}${ex0}${ex0}${ex0}${ex0}${ex0}' .
   '${ex2}${ex2}${ex2}${ex2}${ex2}' .
   '${ex3}${ex3}${ex3}${ex3}${ex3}${ex3}${ex3}${ex3}${ex3}' .
   '${ex4}${ex4}${ex4}${ex4}${ex4}${ex4}${ex4}${ex4}${ex4}' .
   '${ex5}${ex5}${ex5}${ex5}${ex5}${ex5}${ex5}${ex5}${ex5}' .
   '${ex6}${ex6}${ex6}${ex6}${ex6}${ex6}${ex6}${ex6}${ex6}' .
   '${ex7}${ex7}${ex7}${ex7}${ex7}${ex7}${ex7}${ex7}${ex7}' .
   '${ex8}${ex8}${ex8}${ex8}${ex8}${ex8}${ex8}${ex8}${ex8}' .
   "'\n",
   'recursive or exponential expansion is limited');

# Strange input
is($s->substvars('Nothing to $ ${substitute  here}, is it ${}?, it ${is'),
                 'Nothing to $ ${substitute  here}, is it ${}?, it ${is',
                 'substvars strange');

# Warnings about unused variables
$output = '';
$SIG{__WARN__} = sub { $output .= $_[0] };
$s->warn_about_unused();
delete $SIG{__WARN__};
is($output,
   'Dpkg_Substvars.t: warning: test substitution variable ${var2} unused, but is defined' . "\n",
   'unused variables warnings');

# Disable warnings for a certain variable
$s->set_as_used('var_used', 'used');
$s->mark_as_used('var2');
$output = '';
$SIG{__WARN__} = sub { $output .= $_[0] };
$s->warn_about_unused();
delete $SIG{__WARN__};
is($output, '', 'disabled unused variables warnings');

$s->delete('var_used');

# Required variables
my $sr;

$expected = <<'VARS';
required-var!=Required value
VARS
$sr = Dpkg::Substvars->new("$datadir/substvars-req");
is($sr->output(), $expected, 'Required variable preserved');

is($sr->substvars('This is a string with missing the required variable'),
                  'This is a string with missing the required variable',
   'substvars required substitution missing');

eval {
    $sr->warn_about_unused();
    1;
};
$output = $@ // q{};
is($output,
   'Dpkg_Substvars.t: error: required substitution variable ${required-var} not used' . "\n",
   'substvars required substitution not used');

is($sr->substvars('This is a string with a required variable ${required-var}'),
                  'This is a string with a required variable Required value',
    'substvars required substitution present');

# Variable filters
my $sf;

$expected = <<'VARS';
name3=Yet another value
name4=Name value
otherprefix:var7=Quux
var1=Some value
var2=Some other value
VARS
$sf = Dpkg::Substvars->new("$datadir/substvars2");
$sf->filter(remove => sub { $_[0] =~ m/^prefix:/ });
is($sf->output(), $expected, 'Filter remove variables');

$expected = <<'VARS';
otherprefix:var7=Quux
prefix:var5=Foo
var1=Some value
var2=Some other value
VARS
$sf = Dpkg::Substvars->new("$datadir/substvars2");
$sf->filter(keep => sub { $_[0] =~ m/var/ });
is($sf->output(), $expected, 'Filter keep variables');

$expected = <<'VARS';
prefix:name6=Bar
VARS
$sf = Dpkg::Substvars->new("$datadir/substvars2");
$sf->filter(remove => sub { $_[0] =~ m/var/ },
            keep => sub { $_[0] =~ m/^prefix:/ });
is($sf->output(), $expected, 'Filter keep and remove variables');
