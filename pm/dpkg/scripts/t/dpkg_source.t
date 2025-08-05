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
use Test::Dpkg qw(:needs :paths test_neutralize_checksums);

use File::Spec::Functions qw(rel2abs);
use File::Compare;
use File::Path qw(make_path);

use Dpkg::File;
use Dpkg::IPC;
use Dpkg::Substvars;

test_needs_command('xz');

plan tests => 8;

my $srcdir = rel2abs($ENV{srcdir} || '.');
my $datadir = "$srcdir/t/dpkg_source";
my $tmpdir = test_get_temp_path();

$ENV{$_} = rel2abs($ENV{$_}) foreach qw(DPKG_DATADIR DPKG_ORIGINS_DIR);

# Delete variables that can affect the tests.
delete $ENV{SOURCE_DATE_EPOCH};

chdir $tmpdir;

my $tmpl_format = <<'TMPL_FORMAT';
3.0 (native)
TMPL_FORMAT

my $tmpl_changelog = <<'TMPL_CHANGELOG';
${source-name} (${source-version}) ${suite}; urgency=${urgency}

  * Test package.

 -- ${maintainer}  Sat, 05 Jul 2014 21:11:22 +0200
TMPL_CHANGELOG

my $tmpl_control = <<'TMPL_CONTROL';
Source: ${source-name}
Section: ${source-section}
Priority: ${source-priority}
Maintainer: ${maintainer}
Standards-Version: 1.0
Testsuite: ${source-testsuite}

Package: test-binary
Architecture: all
Description: test package
TMPL_CONTROL

my $tmpl_control_tests = <<'TMPL_CONTROL_TESTS';
Test-Command: test-unique
Depends: @, aa

Tests: test-dupe
Depends: @builddeps@

Test-Command: test-dupe
Depends: bb, test-binary

Test-Command: test-dupe
Depends: cc
TMPL_CONTROL_TESTS

my %default_substvars = (
    'source-name' => 'test-source',
    'source-version' => 0,
    'source-section' => 'test',
    'source-priority' => 'optional',
    'source-testsuite' => 'autopkgtest',
    'suite' => 'unstable',
    'urgency' => 'low',
    'maintainer' => 'Dpkg Developers <debian-dpkg@lists.debian.org>',
);

sub gen_from_tmpl
{
    my ($pathname, $tmpl, $substvars) = @_;

    file_dump($pathname, $substvars->substvars($tmpl));
}

sub gen_source
{
    my (%opts) = @_;

    my $substvars = Dpkg::Substvars->new();
    foreach my $var ((keys %default_substvars, keys %opts)) {
        my $value = $opts{$var} // $default_substvars{$var};

        $substvars->set_as_auto($var, $value);
    }

    my $source = $substvars->get('source-name');
    my $version = $substvars->get('source-version');
    my $dirname = "$source-$version";

    make_path("$dirname/debian/source");

    gen_from_tmpl("$dirname/debian/source/format", $tmpl_format, $substvars);
    gen_from_tmpl("$dirname/debian/changelog", $tmpl_changelog, $substvars);
    gen_from_tmpl("$dirname/debian/control", $tmpl_control, $substvars);

    if (defined $opts{'control-test'}) {
        make_path("$dirname/debian/tests");
        gen_from_tmpl("$dirname/debian/tests/control", $opts{'control-test'}, $substvars);
    }

    return $dirname;
}

sub test_diff
{
    my $filename = shift;

    my $expected_file = "$datadir/$filename";
    my $generated_file =  $filename;

    test_neutralize_checksums($generated_file);

    my $res = compare($expected_file, $generated_file);
    if ($res) {
        system "diff -u $expected_file $generated_file >&2";
    }
    ok($res == 0, "generated file matches expected one ($expected_file)");
}

sub test_build_source
{
    my ($name) = shift;
    my $stderr;

    spawn(exec => [ $ENV{PERL}, "$srcdir/dpkg-source.pl", '--build', $name ],
          error_to_string => \$stderr,
          wait_child => 1, nocheck => 1);

    ok($? == 0, 'dpkg-source --build succeeded');
    diag($stderr) unless $? == 0;

    my $basename = $name =~ tr/-/_/r;

    test_diff("$basename.dsc");
}

my $dirname;

$dirname = gen_source('source-name' => 'testsuite',
                      'source-version' => 0,
                      'control-test' => '');
test_build_source($dirname);

$dirname = gen_source('source-name' => 'testsuite',
                      'source-version' => 1,
                      'control-test' => '');
test_build_source($dirname);

$dirname = gen_source('source-name' => 'testsuite',
                      'source-version' => 2,
                      'source-testsuite' => 'smokepkgtest, unitpkgtest, funcpkgtest',
                      'control-test' => $tmpl_control_tests);
test_build_source($dirname);

$dirname = gen_source('source-name' => 'testsuite',
                      'source-version' => 3);
test_build_source($dirname);
