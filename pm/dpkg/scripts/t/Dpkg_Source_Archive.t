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
use Test::Dpkg qw(:paths);

use File::Spec;
use File::Path qw(make_path);

BEGIN {
    use_ok('Dpkg::Source::Archive');
}

use Dpkg;
use Dpkg::File;

my $tmpdir = test_get_temp_path();

sub test_path_escape
{
    my $name = shift;

    my $treedir = File::Spec->rel2abs("$tmpdir/$name-tree");
    my $overdir = File::Spec->rel2abs("$tmpdir/$name-overlay");
    my $outdir = "$tmpdir/$name-out";
    my $expdir = "$tmpdir/$name-exp";

    # This is the base directory, where we are going to be extracting stuff
    # into, which include traps.
    make_path("$treedir/subdir-a");
    file_touch("$treedir/subdir-a/file-a");
    file_touch("$treedir/subdir-a/file-pre-a");
    make_path("$treedir/subdir-b");
    file_touch("$treedir/subdir-b/file-b");
    file_touch("$treedir/subdir-b/file-pre-b");
    symlink File::Spec->abs2rel($outdir, $treedir), "$treedir/symlink-escape";
    symlink File::Spec->abs2rel("$outdir/nonexistent", $treedir), "$treedir/symlink-nonexistent";
    symlink "$treedir/file", "$treedir/symlink-within";
    file_touch("$treedir/supposed-dir");

    # This is the overlay directory, which we'll pack and extract over the
    # base directory.
    make_path($overdir);
    make_path("$overdir/subdir-a/aa");
    file_dump("$overdir/subdir-a/aa/file-aa", 'aa');
    file_dump("$overdir/subdir-a/file-a", 'a');
    make_path("$overdir/subdir-b/bb");
    file_dump("$overdir/subdir-b/bb/file-bb", 'bb');
    file_dump("$overdir/subdir-b/file-b", 'b');
    make_path("$overdir/symlink-escape");
    file_dump("$overdir/symlink-escape/escaped-file", 'escaped');
    file_dump("$overdir/symlink-nonexistent", 'nonexistent');
    make_path("$overdir/symlink-within");
    make_path("$overdir/supposed-dir");
    file_dump("$overdir/supposed-dir/supposed-file", 'something');

    # Generate overlay tar.
    system($Dpkg::PROGTAR, '-cf', "$overdir.tar", '-C', $overdir, qw(
        subdir-a subdir-b
        symlink-escape/escaped-file symlink-nonexistent symlink-within
        supposed-dir
        )) == 0
        or die "cannot create overlay tar archive\n";

   # This is the expected directory, which we'll be comparing against.
    make_path($expdir);
    system('cp', '-a', $overdir, $expdir) == 0
        or die "cannot copy overlay hierarchy into expected directory\n";

    # Store the expected and out reference directories into a tar to compare
    # its structure against the result reference.
    system($Dpkg::PROGTAR, '-cf', "$expdir.tar", '-C', $overdir, qw(
        subdir-a subdir-b
        symlink-escape/escaped-file symlink-nonexistent symlink-within
        supposed-dir
        ), '-C', $treedir, qw(
        subdir-a/file-pre-a
        subdir-b/file-pre-b
        )) == 0
        or die "cannot create expected tar archive\n";

    # This directory is supposed to remain empty, anything inside implies a
    # directory traversal.
    make_path($outdir);

    my $warnseen;
    local $SIG{__WARN__} = sub { $warnseen = $_[0] };

    # Perform the extraction.
    my $tar = Dpkg::Source::Archive->new(filename => "$overdir.tar");
    $tar->extract($treedir, in_place => 1);

    # Store the result into a tar to compare its structure against a reference.
    system($Dpkg::PROGTAR, '-cf', "$treedir.tar", '-C', $treedir, '.');

    # Check results
    ok(length $warnseen && $warnseen =~ m/points outside source root/,
       'expected warning seen');
    ok(system($Dpkg::PROGTAR, '--compare', '-f', "$expdir.tar", '-C', $treedir) == 0,
       'expected directory matches');
    ok(! -e "$outdir/escaped-file",
       'expected output directory is empty, directory traversal');
}

test_path_escape('in-place');

# TODO: Add actual test cases.
