#!/usr/bin/perl
#
# Copyright © 2016 Guillem Jover <guillem@debian.org>
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
use version;

use Test::More tests => 6;
use Cwd;
use File::Path qw(make_path remove_tree);
use File::Temp qw(tempdir);
use File::Basename;
use File::Find;

use Dpkg::File;
use Dpkg::IPC;

my $srcdir = $ENV{srcdir} || '.';
my $builddir = $ENV{builddir} || '.';
my $tmpdir = 't.tmp/t-treewalk';

# Set a known umask.
umask 0022;

# Populate the tree hierarchy.
sub make_tree {
    my ($dirtree) = @_;
    my $cwd = getcwd();

    make_path($dirtree);
    chdir $dirtree;

    # Deep tree.
    make_path('aaaa/aaaa/aaaa/aaaa/');
    file_touch('aaaa/aaaa/aaaa/aaaa/abcde');
    file_touch('aaaa/aaaa/aaaa/aaaa/ddddd');
    file_touch('aaaa/aaaa/aaaa/aaaa/wwwwa');
    file_touch('aaaa/aaaa/aaaa/aaaa/wwwwz');
    file_touch('aaaa/aaaa/aaaa/aaaa/zzzzz');

    # Shallow tree.
    make_path('bbbb/');
    file_touch('bbbb/abcde');
    file_touch('bbbb/ddddd');
    file_touch('bbbb/wwwwa');
    file_touch('bbbb/wwwwz');
    file_touch('bbbb/zzzzz');

    # Populated tree.
    make_path('cccc/aa/aa/aa/');
    make_path('cccc/aa/aa/bb/aa/');
    file_touch('cccc/aa/aa/bb/aa/file-a');
    file_touch('cccc/aa/aa/bb/aa/file-z');
    make_path('cccc/aa/bb/');
    make_path('cccc/bb/aa/');
    make_path('cccc/bb/bb/aa/aa/');
    file_touch('cccc/bb/bb/aa/aa/file-a');
    file_touch('cccc/bb/bb/aa/aa/file-z');
    make_path('cccc/bb/bb/bb/');
    file_touch('cccc/bb/bb/bb/file-w');
    make_path('cccc/cc/aa/');
    make_path('cccc/cc/bb/aa/');
    file_touch('cccc/cc/bb/aa/file-t');
    make_path('cccc/cc/bb/bb/');
    file_touch('cccc/cc/bb/bb/file-x');
    make_path('cccc/cc/cc/');
    make_path('cccc/dd/aa/aa/aa/');
    file_touch('cccc/dd/aa/aa/aa/file-y');
    make_path('cccc/dd/aa/aa/bb/');
    file_touch('cccc/dd/aa/aa/bb/file-o');
    make_path('cccc/dd/aa/bb/aa/');
    file_touch('cccc/dd/aa/bb/aa/file-k');
    make_path('cccc/dd/aa/bb/bb/');
    file_touch('cccc/dd/aa/bb/bb/file-l');
    make_path('cccc/dd/aa/cc/aa/');
    file_touch('cccc/dd/aa/cc/aa/file-s');
    make_path('cccc/dd/aa/cc/bb/');
    file_touch('cccc/dd/aa/cc/bb/file-u');

    # Tree with symlinks cycles.
    make_path('llll/self/');
    file_touch('llll/file');
    symlink '..', 'llll/self/loop';
    make_path('llll/real/');
    file_touch('llll/real/file-r');
    symlink '../virt', 'llll/real/loop';
    make_path('llll/virt/');
    file_touch('llll/virt/file-v');
    symlink '../real', 'llll/virt/loop';

    chdir $cwd;
}

sub test_treewalker {
    my $stdout;
    my $stderr;
    my $dirtree = $tmpdir;

    # Check generated tarballs.
    foreach my $type (qw(full skip)) {
        my @paths;

        make_tree($dirtree);

        find({ no_chdir => 1, wanted => sub {
                   return if $type eq 'skip' and m{^\Q$dirtree\E/cccc};
                   push @paths, s{\./}{}r;
               },
             }, $dirtree);

        my $expected;

        foreach my $path (sort @paths) {
            lstat $path;

            my $ptype;
            if (-f _) {
                $ptype = 'f';
            } elsif (-l _) {
                $ptype = 'l';
            } elsif (-d _) {
                $ptype = 'd';
            }
            my $pname = basename($path);
            my $pvirt = $path =~ s{\Q$dirtree\E/?}{}r;

            $expected .= "T=$ptype N=$pname V=$pvirt R=$path\n";
        }

        $ENV{TREEWALK_SKIP} = $type eq 'skip' ? "$dirtree/cccc" : undef;

        spawn(exec => [ "$builddir/t/c-treewalk", $dirtree ],
              nocheck => 1, to_string => \$stdout, to_error => \$stderr);
        ok($? == 0, "tree walker $type should succeed");
        is($stderr, undef, "tree walker $type stderr is empty");
        is($stdout, $expected, "tree walker $type is ok");
    }
}

test_treewalker();
