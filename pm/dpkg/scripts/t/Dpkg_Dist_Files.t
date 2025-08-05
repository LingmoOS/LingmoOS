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

use Test::More tests => 26;
use Test::Dpkg qw(:paths);

use_ok('Dpkg::Dist::Files');

my $datadir = test_get_data_path();

my $expected;
my %expected = (
    'pkg-src_4:2.0+1A~rc1-1.dsc' => {
        filename => 'pkg-src_4:2.0+1A~rc1-1.dsc',
        section => 'source',
        priority => 'extra',
        attrs => {},
    },
    'pkg-src_4:2.0+1A~rc1-1.tar.xz' => {
        filename => 'pkg-src_4:2.0+1A~rc1-1.tar.xz',
        section => 'source',
        priority => 'extra',
        attrs => {},
    },
    'pkg-templ_1.2.3_arch.type' => {
        filename => 'pkg-templ_1.2.3_arch.type',
        package => 'pkg-templ',
        package_type => 'type',
        version => '1.2.3',
        arch => 'arch',
        section => 'section',
        priority => 'priority',
        attrs => {},
    },
    'pkg-arch_2.0.0_amd64.deb' => {
        filename => 'pkg-arch_2.0.0_amd64.deb',
        package => 'pkg-arch',
        package_type => 'deb',
        version => '2.0.0',
        arch => 'amd64',
        section => 'admin',
        priority => 'required',
        attrs => {},
    },
    'pkg-indep_0.0.1-2_all.deb' => {
        filename => 'pkg-indep_0.0.1-2_all.deb',
        package => 'pkg-indep',
        package_type => 'deb',
        version => '0.0.1-2',
        arch => 'all',
        section => 'net',
        priority => 'standard',
        attrs => {},
    },
    'other_0.txt' => {
        filename => 'other_0.txt',
        section => 'text',
        priority => 'optional',
        attrs => {
            'mime-type' => 'text/plain',
        },
    },
    'BY-HAND-file' => {
        filename => 'BY-HAND-file',
        section => 'webdocs',
        priority => 'optional',
        attrs => {
            'by-hand' => 'true'
        },
    },
    'another:filename' => {
        filename => 'another:filename',
        section => 'by-hand',
        priority => 'extra',
        attrs => {},
    },
    'added-on-the-fly' => {
        filename => 'added-on-the-fly',
        section => 'void',
        priority => 'wish',
        attrs => {},
    },
);

my $dist = Dpkg::Dist::Files->new();
$dist->load("$datadir/files-byhand") or error('cannot parse file');

$expected = <<'FILES';
BY-HAND-file webdocs optional by-hand=true
other_0.txt text optional mime-type=text/plain
pkg-arch_2.0.0_amd64.deb admin required
pkg-indep_0.0.1-2_all.deb net standard
pkg-templ_1.2.3_arch.type section priority
FILES

is($dist->output(), $expected, 'Parsed dist file');
foreach my $f ($dist->get_files()) {
    my $filename = $f->{filename};

    is_deeply($f, $expected{$filename},
              "Detail for individual dist file $filename, via get_files()");

    my $fs = $dist->get_file($filename);
    is_deeply($fs, $expected{$filename},
              "Detail for individual dist file $filename, via get_file()");
}

is($dist->parse_filename('file%invalid'), undef, 'invalid filename');

$expected = <<'FILES';
BY-HAND-file webdocs optional by-hand=true
added-on-the-fly void wish
other_0.txt text optional mime-type=text/plain
pkg-arch_2.0.0_amd64.deb void imperative
pkg-templ_1.2.3_arch.type section priority
FILES

$dist->add_file('added-on-the-fly', 'void', 'wish');
is_deeply($dist->get_file('added-on-the-fly'), $expected{'added-on-the-fly'},
    'Get added file added-on-the-fly');

$dist->add_file('pkg-arch_2.0.0_amd64.deb', 'void', 'imperative');
my %expected_pkg_arch = %{$expected{'pkg-arch_2.0.0_amd64.deb'}};
$expected_pkg_arch{section} = 'void';
$expected_pkg_arch{priority} = 'imperative';
is_deeply($dist->get_file('pkg-arch_2.0.0_amd64.deb'), \%expected_pkg_arch,
    'Get modified file pkg-arch_2.0.0_amd64.deb');

$dist->del_file('pkg-indep_0.0.1-2_all.deb');
is($dist->get_file('unknown'), undef, 'Get unknown file');
is($dist->get_file('pkg-indep_0.0.1-2_all.deb'), undef, 'Get deleted file');
is($dist->output(), $expected, 'Modified dist object');

$expected = <<'FILES';
another:filename by-hand extra
pkg-src_4:2.0+1A~rc1-1.dsc source extra
pkg-src_4:2.0+1A~rc1-1.tar.xz source extra
FILES

$dist->reset();
$dist->add_file('pkg-src_4:2.0+1A~rc1-1.dsc', 'source', 'extra');
$dist->add_file('pkg-src_4:2.0+1A~rc1-1.tar.xz', 'source', 'extra');
$dist->add_file('another:filename', 'by-hand', 'extra');

is_deeply($dist->get_file('pkg-src_4:2.0+1A~rc1-1.dsc'),
          $expected{'pkg-src_4:2.0+1A~rc1-1.dsc'},
          'Get added file pkg-src_4:2.0+1A~rc1-1.dsc');
is_deeply($dist->get_file('pkg-src_4:2.0+1A~rc1-1.tar.xz'),
          $expected{'pkg-src_4:2.0+1A~rc1-1.tar.xz'},
          'Get added file pkg-src_4:2.0+1A~rc1-1.tar.xz');
is_deeply($dist->get_file('another:filename'),
          $expected{'another:filename'},
          'Get added file another:filename');
is($dist->output, $expected, 'Added source files');

$expected = <<'FILES';
BY-HAND-file webdocs optional by-hand=true
other_0.txt text optional mime-type=text/plain
pkg-arch_2.0.0_amd64.deb admin required
pkg-frag-a_0.0_arch.type section priority
pkg-frag-b_0.0_arch.type section priority
pkg-indep_0.0.1-2_all.deb net standard
pkg-templ_1.2.3_arch.type section priority
FILES

$dist->reset();
$dist->load_dir($datadir) or error('cannot parse fragment files');
is($dist->output(), $expected, 'Parse fragment directory');

$expected = <<'FILES';
pkg-arch_2.0.0_amd64.deb admin required
pkg-indep_0.0.1-2_all.deb net standard
pkg-templ_1.2.3_arch.type section priority
FILES

$dist->reset();
$dist->load("$datadir/files-byhand") or error('cannot parse file');
$dist->filter(remove => sub { $_[0]->{priority} eq 'optional' });
is($dist->output(), $expected, 'Filter remove priority optional');

$expected = <<'FILES';
BY-HAND-file webdocs optional by-hand=true
other_0.txt text optional mime-type=text/plain
FILES

$dist->reset();
$dist->load("$datadir/files-byhand") or error('cannot parse file');
$dist->filter(keep => sub { $_[0]->{priority} eq 'optional' });
is($dist->output(), $expected, 'Filter keep priority optional');

$expected = <<'FILES';
BY-HAND-file webdocs optional by-hand=true
FILES

$dist->reset();
$dist->load("$datadir/files-byhand") or error('cannot parse file');
$dist->filter(remove => sub { $_[0]->{section} eq 'text' },
              keep => sub { $_[0]->{priority} eq 'optional' });
is($dist->output(), $expected, 'Filter remove section text, keep priority optional');
