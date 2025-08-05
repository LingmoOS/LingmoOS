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

use Test::More tests => 11;
use Test::Dpkg qw(:paths);

use Cwd qw(realpath);
use File::Path qw(make_path rmtree);
use File::Spec::Functions qw(abs2rel);

use Dpkg::File;

use_ok('Dpkg::BuildTree');

my $tmpdir = test_get_temp_path();

make_path("$tmpdir/debian/tmp");

my @srcfiles = qw(
    changelog
    control
    copyright
    shlibs.local
);

my @genfiles = qw(
    files
    files.new
    substvars
    substvars.new
    tmp/build-object
    tmp/generated
);

foreach my $file (@srcfiles, @genfiles) {
    file_touch("$tmpdir/debian/$file");
}

my $bt = Dpkg::BuildTree->new(dir => $tmpdir);

$bt->clean();

foreach my $srcfile (@srcfiles) {
    ok(-e "$tmpdir/debian/$srcfile", "source file $tmpdir/$srcfile exists");
}

foreach my $genfile (@genfiles) {
    ok(! -e "$tmpdir/debian/$genfile", "generated file $tmpdir/$genfile does not exist");
}
