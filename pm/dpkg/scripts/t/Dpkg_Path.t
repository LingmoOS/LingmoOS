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

use Test::More tests => 34;
use Test::Dpkg qw(:paths);

use Cwd qw(realpath);
use File::Path qw(make_path rmtree);
use File::Spec::Functions qw(abs2rel);

use Dpkg::File;

use_ok('Dpkg::Path', 'canonpath', 'resolve_symlink',
       'check_files_are_the_same',
       'check_directory_traversal',
       'get_pkg_root_dir',
       'guess_pkg_root_dir', 'relative_to_pkg_root');

my $tmpdir = test_get_temp_path();

make_path("$tmpdir/a/b/c");
make_path("$tmpdir/a/DEBIAN");
make_path("$tmpdir/debian/a/b/c");
symlink 'a/b/c', "$tmpdir/cbis";
symlink '/this/does/not/exist', "$tmpdir/tmp";
symlink '.', "$tmpdir/here";

is(canonpath("$tmpdir/./a///b/c"), "$tmpdir/a/b/c", 'canonpath basic test');
is(canonpath("$tmpdir/a/b/../../a/b/c"), "$tmpdir/a/b/c", 'canonpath and ..');
is(canonpath("$tmpdir/a/b/c/../../"), "$tmpdir/a", 'canonpath .. at end');
is(canonpath("$tmpdir/cbis/../"), "$tmpdir/cbis/..", 'canonpath .. after symlink');

is(resolve_symlink("$tmpdir/here/cbis"), "$tmpdir/here/a/b/c", 'resolve_symlink');
is(resolve_symlink("$tmpdir/tmp"), '/this/does/not/exist', 'resolve_symlink absolute');
is(resolve_symlink("$tmpdir/here"), $tmpdir, 'resolve_symlink .');

ok(!check_files_are_the_same("$tmpdir/here", $tmpdir), 'Symlink is not the same!');
ok(check_files_are_the_same("$tmpdir/here/a", "$tmpdir/a"), 'Same directory');

sub gen_hier_travbase {
    my $basedir = shift;

    make_path("$basedir/subdir");
    file_touch("$basedir/file");
    file_touch("$basedir/subdir/subfile");
    symlink 'file', "$basedir/symlink-file";
    symlink 'subdir/subfile', "$basedir/symlink-subfile";
}

my $travbase = "$tmpdir/travbase";
my $travbase_out = "$tmpdir/travbase-out";
my %travtype = (
    none => {
        fail => 0,
        gen => sub { },
    },
    same => {
        fail => 0,
        chroot => "$tmpdir/travbase-same",
        gen => sub {
            my $basedir = shift;
            symlink '../..', "$basedir/subdir/root";
        },
    },
    dev_null => {
        fail => 0,
        gen => sub {
            my $basedir = shift;
            symlink '/dev/null', "$basedir/dev-null";
        },
    },
    dots => {
        fail => 0,
        gen => sub {
            my $basedir = shift;
            symlink 'aa..bb..cc', "$basedir/dots";
        },
    },
    rel => {
        fail => 1,
        gen => sub {
            my $basedir = shift;
            symlink '../../..', "$basedir/rel";
        },
    },
    abs => {
        fail => 1,
        gen => sub {
            my $basedir = shift;
            symlink '/etc', "$basedir/abs";
        },
    },
    loop => {
        fail => 1,
        gen => sub {
            my $basedir = shift;
            symlink 'self', "$basedir/self";
        },
    },
    enoent_rel => {
        fail => 0,
        gen => sub {
            my $basedir = shift;
            symlink 'not-existent', "$basedir/enoent-rel";
        },
    },
    enoent_abs => {
        fail => 1,
        gen => sub {
            my $basedir = shift;
            symlink '/not-existent', "$basedir/enoent-abs";
        },
    },
    enoent_indirect_rel => {
        fail => 0,
        gen => sub {
            my $basedir = shift;
            symlink 'not-existent', "$basedir/enoent-rel";
            symlink 'enoent-rel', "$basedir/enoent-indirect-rel";
        },
    },
    enoent_indirect_abs => {
        fail => 1,
        gen => sub {
            my $basedir = shift;
            symlink '/not-existent', "$basedir/enoent-abs";
            symlink realpath("$basedir/enoent-abs"), "$basedir/enoent-indirect-abs";
        },
    },
    base_in_none => {
        fail => 0,
        gen => sub {
            my $basedir = shift;
            rename $basedir, "$basedir-real";
            symlink 'base_in_none-real', $basedir;
        },
    },
    base_in_rel => {
        fail => 1,
        gen => sub {
            my $basedir = shift;
            rename $basedir, "$basedir-real";
            symlink 'base_in_rel-real', $basedir;
            symlink '../../..', "$basedir/rel";
        },
    },
    base_in_abs => {
        fail => 1,
        gen => sub {
            my $basedir = shift;
            rename $basedir, "$basedir-real";
            symlink 'base_in_abs-real', $basedir;
            symlink '/etc', "$basedir/abs";
        },
    },
    base_out_empty => {
        fail => 1,
        root => $travbase_out,
        gen => sub {
            my $basedir = shift;
            rmtree($basedir);
            make_path($basedir);
        },
    },
    base_out_none => {
        fail => 1,
        root => $travbase_out,
        gen => sub { },
    },
    base_out_rel => {
        fail => 1,
        root => $travbase_out,
        gen => sub {
            my $basedir = shift;
            symlink '../../..', "$basedir/rel";
        },
    },
    base_out_abs => {
        fail => 1,
        root => $travbase_out,
        gen => sub {
            my $basedir = shift;
            symlink '/etc', "$basedir/abs";
        },
    },
);

foreach my $travtype (sort keys %travtype) {
    my $trav = $travtype{$travtype};
    my $rootdir = $trav->{chroot} // $trav->{root} // $travbase;
    my $hierdir = "$rootdir/$travtype";
    my $travdir = "$travbase/$travtype";

    gen_hier_travbase($hierdir);
    symlink abs2rel($hierdir, $travbase), $travdir if exists $trav->{root};
    $trav->{gen}->($hierdir);

    my $catch;
    eval {
        check_directory_traversal($travbase, $travdir);
        1;
    } or do {
        $catch = $@;
    };
    if ($trav->{fail}) {
        ok($catch, "directory traversal type $travtype detected");
        note("traversal reason: $catch") if $catch;
    } else {
        ok(! $catch, "no directory traversal type $travtype");
        diag("error from check_directory_traversal => $catch") if $catch;
    }
}

is(get_pkg_root_dir("$tmpdir/a/b/c"), "$tmpdir/a", 'get_pkg_root_dir');
is(guess_pkg_root_dir("$tmpdir/a/b/c"), "$tmpdir/a", 'guess_pkg_root_dir');
is(relative_to_pkg_root("$tmpdir/a/b/c"), 'b/c', 'relative_to_pkg_root');

chdir($tmpdir);

is(get_pkg_root_dir('debian/a/b/c'), undef, 'get_pkg_root_dir undef');
is(relative_to_pkg_root('debian/a/b/c'), undef, 'relative_to_pkg_root undef');
is(guess_pkg_root_dir('debian/a/b/c'), 'debian/a', 'guess_pkg_root_dir fallback');
