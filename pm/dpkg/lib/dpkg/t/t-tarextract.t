#!/usr/bin/perl
#
# Copyright © 2014 Guillem Jover <guillem@debian.org>
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

use Test::More;
use Cwd;
use File::Path qw(make_path remove_tree);
use File::Temp qw(tempdir);
use File::Spec;
use File::Find;
use POSIX qw(mkfifo);

use Dpkg ();
use Dpkg::File;
use Dpkg::IPC;

use strict;
use warnings;
use version;

my $srcdir = $ENV{srcdir} || '.';
my $builddir = $ENV{builddir} || '.';
my $tmpdir = 't.tmp/t-tarextract';

# We require GNU tar >= 1.27 for --owner=NAME:ID and --group=NAME:ID.
my $tar_version = qx($Dpkg::PROGTAR --version 2>/dev/null);
if ($tar_version and $tar_version =~ m/^tar \(GNU tar\) (\d+\.\d+)/ and
    qv("v$1") >= qv('v1.27'))
{
    plan tests => 12;
} else {
    plan skip_all => 'needs GNU tar >= 1.27';
}

# Set a known umask.
umask 0022;

sub tar_create_tree {
    my $type = shift;

    my $long_a = 'a' x 29;
    my $long_b = 'b' x 29;
    my $long_c = 'c' x 29;
    my $long_d = 'd' x 29;
    my $long_e = 'e' x 29;
    my $long_f = 'f' x 22;

    # Populate tar hierarchy
    file_touch('file');
    link 'file', 'hardlink';

    make_path("$long_a/$long_b/$long_c/$long_d/$long_e/");
    make_path("$long_a/$long_b/$long_c/$long_d/$long_e/$long_f/");
    file_touch("$long_a/$long_b/$long_c/$long_d/$long_e/$long_f/long");

    # POSIX specifies that symlinks have undefined permissions in their
    # mode, so their handling is system dependent. Linux does not honor
    # the umask for symlinks, other systems like GNU/Hurd or kFreeBSD do,
    # which means we get different results due to this.
    my $umask = umask 0;

    symlink "$long_a/$long_b/$long_c/$long_d/$long_e/$long_f/long",
            'symlink-long';
    symlink 'file', 'symlink-a';
    symlink 'hardlink', 'symlink-b';
    symlink 'dangling', 'symlink-c';

    umask $umask;

    mkdir 'directory';
    mkfifo('fifo', 0770);

    # TODO: Need root.
    # system 'mknod', 'chardev', 'c', '1', '3';
    # system 'mknod', 'blockdev', 'b', '0', '0';
}

sub test_tar_extractor {
    my $stdout;
    my $stderr;

    my $expected_tar = <<'TAR';
. mode=40755 time=100000000.000000000 uid=100 gid=200 uname=user gname=group type=dir
./fifo mode=10750 time=100000000.000000000 uid=100 gid=200 uname=user gname=group type=fifo
./file mode=100644 time=100000000.000000000 uid=100 gid=200 uname=user gname=group type=file size=0
./hardlink mode=100644 time=100000000.000000000 uid=100 gid=200 uname=user gname=group type=hardlink linkto=./file size=0
./aaaaaaaaaaaaaaaaaaaaaaaaaaaaa mode=40755 time=100000000.000000000 uid=100 gid=200 uname=user gname=group type=dir
./aaaaaaaaaaaaaaaaaaaaaaaaaaaaa/bbbbbbbbbbbbbbbbbbbbbbbbbbbbb mode=40755 time=100000000.000000000 uid=100 gid=200 uname=user gname=group type=dir
./aaaaaaaaaaaaaaaaaaaaaaaaaaaaa/bbbbbbbbbbbbbbbbbbbbbbbbbbbbb/ccccccccccccccccccccccccccccc mode=40755 time=100000000.000000000 uid=100 gid=200 uname=user gname=group type=dir
./aaaaaaaaaaaaaaaaaaaaaaaaaaaaa/bbbbbbbbbbbbbbbbbbbbbbbbbbbbb/ccccccccccccccccccccccccccccc/ddddddddddddddddddddddddddddd mode=40755 time=100000000.000000000 uid=100 gid=200 uname=user gname=group type=dir
./aaaaaaaaaaaaaaaaaaaaaaaaaaaaa/bbbbbbbbbbbbbbbbbbbbbbbbbbbbb/ccccccccccccccccccccccccccccc/ddddddddddddddddddddddddddddd/eeeeeeeeeeeeeeeeeeeeeeeeeeeee mode=40755 time=100000000.000000000 uid=100 gid=200 uname=user gname=group type=dir
./aaaaaaaaaaaaaaaaaaaaaaaaaaaaa/bbbbbbbbbbbbbbbbbbbbbbbbbbbbb/ccccccccccccccccccccccccccccc/ddddddddddddddddddddddddddddd/eeeeeeeeeeeeeeeeeeeeeeeeeeeee/ffffffffffffffffffffff mode=40755 time=100000000.000000000 uid=100 gid=200 uname=user gname=group type=dir
./aaaaaaaaaaaaaaaaaaaaaaaaaaaaa/bbbbbbbbbbbbbbbbbbbbbbbbbbbbb/ccccccccccccccccccccccccccccc/ddddddddddddddddddddddddddddd/eeeeeeeeeeeeeeeeeeeeeeeeeeeee/ffffffffffffffffffffff/long mode=100644 time=100000000.000000000 uid=100 gid=200 uname=user gname=group type=file size=0
./directory mode=40755 time=100000000.000000000 uid=100 gid=200 uname=user gname=group type=dir
./symlink-a mode=120777 time=100000000.000000000 uid=100 gid=200 uname=user gname=group type=symlink linkto=file size=0
./symlink-b mode=120777 time=100000000.000000000 uid=100 gid=200 uname=user gname=group type=symlink linkto=hardlink size=0
./symlink-c mode=120777 time=100000000.000000000 uid=100 gid=200 uname=user gname=group type=symlink linkto=dangling size=0
./symlink-long mode=120777 time=100000000.000000000 uid=100 gid=200 uname=user gname=group type=symlink linkto=aaaaaaaaaaaaaaaaaaaaaaaaaaaaa/bbbbbbbbbbbbbbbbbbbbbbbbbbbbb/ccccccccccccccccccccccccccccc/ddddddddddddddddddddddddddddd/eeeeeeeeeeeeeeeeeeeeeeeeeeeee/ffffffffffffffffffffff/long size=0
TAR

    make_path($tmpdir);

    my $cwd = getcwd();

    # Check generated tarballs.
    foreach my $type (qw(v7 ustar oldgnu gnu)) {
        my $dirtree = "$tmpdir/$type";
        my @paths;

        mkdir $dirtree;
        chdir $dirtree;
        tar_create_tree($type);
        find({ no_chdir => 1, wanted => sub {
                   return if $type eq 'v7' and length > 99;
                   return if $type eq 'v7' and -l and length readlink > 99;
                   return if $type eq 'v7' and not (-f or -l or -d);
                   return if $type eq 'ustar' and length > 256;
                   return if $type eq 'ustar' and -l and length readlink > 100;
                   push @paths, $_;
               },
               preprocess => sub { my (@files) = sort @_; @files } }, '.');
        chdir $cwd;

        my $paths_list = join "\0", @paths;
        spawn(exec => [ $Dpkg::PROGTAR, '-cf', "$dirtree.tar",
                        '--format', $type,
                        '-C', $dirtree, '--mtime=@100000000',
                        '--owner=user:100', '--group=group:200',
                        '--null', '--no-unquote', '--no-recursion', '-T-' ],
              wait_child => 1, from_string => \$paths_list);

        my $expected = $expected_tar;
        $expected =~ s/[ug]name=[^ ]+ //g if $type eq 'v7';
        $expected =~ s/\n^.*fifo.*$//mg if $type eq 'v7';
        $expected =~ s/\n^.*dddd.*$//mg if $type eq 'v7';
        $expected =~ s/\n^.*symlink-long.*$//mg if $type eq 'ustar';

        spawn(exec => [ "$builddir/t/c-tarextract", "$dirtree.tar" ],
              nocheck => 1, to_string => \$stdout, to_error => \$stderr);
        ok($? == 0, "tar extractor $type should succeed");
        is($stderr, undef, "tar extractor $type stderr is empty");
        is($stdout, $expected, "tar extractor $type is ok");
    }
}

test_tar_extractor();
