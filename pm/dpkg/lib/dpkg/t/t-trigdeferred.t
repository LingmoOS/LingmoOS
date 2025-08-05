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

use Test::More;
use Cwd;
use File::Path qw(make_path remove_tree);
use File::Temp qw(tempdir);
use File::Basename;
use File::Find;

use Dpkg::File;
use Dpkg::IPC;

my $srcdir = $ENV{srcdir} || '.';
my $builddir = $ENV{builddir} || '.';
my $tmpdir = 't.tmp/t-trigdeferred';

my @deferred = (
    {
        exitcode => 0,
        original => <<'TEXT',
	    
    # Comment	
	# Another Comment
    /root/pathname/file-trigger	    pkg-a pkg-b	 pkg-c
named-trigger   pkg-1    pkg-2	    pkg-3
parse-trigger pkg:a pkg+b pkg.0 0-pkg
:other-trigger      -
TEXT
        expected => <<'TEXT',
<T='/root/pathname/file-trigger' P='pkg-a' P='pkg-b' P='pkg-c' E>
<T='named-trigger' P='pkg-1' P='pkg-2' P='pkg-3' E>
<T='parse-trigger' P='pkg:a' P='pkg+b' P='pkg.0' P='0-pkg' E>
<T=':other-trigger' P='-' E>
TEXT
    }, {
        exitcode => 2,
        original => <<"TEXT",
\b # invalid character
TEXT
    }, {
        exitcode => 2,
        original => <<'TEXT',
trigger -pkg
TEXT
    }, {
        exitcode => 2,
        original => <<'TEXT',
trigger +pkg
TEXT
    }, {
        exitcode => 2,
        original => <<'TEXT',
trigger .pkg
TEXT
    }, {
        exitcode => 2,
        original => <<'TEXT',
trigger :pkg
TEXT
    }, {
        exitcode => 2,
        original => 'missing newline',
    }
);

plan tests => scalar(@deferred) * 3;

# Set a known umask.
umask 0022;

sub test_trigdeferred {
    my $stdout;
    my $stderr;
    my $admindir = "$tmpdir";

    # Check triggers deferred file parsing.
    make_path("$admindir/triggers");

    foreach my $test (@deferred) {
        file_dump("$admindir/triggers/Unincorp", $test->{original});

        spawn(exec => [ "$builddir/t/c-trigdeferred", $admindir ],
              nocheck => 1, to_string => \$stdout, error_to_string => \$stderr);
        my $exitcode = $? >> 8;

        is($exitcode, $test->{exitcode}, 'trigger deferred expected exitcode');
        if ($test->{exitcode} == 0) {
            is($stderr, '', 'trigger deferred expected stderr');
            is($stdout, $test->{expected}, 'trigger deferred expected stdout');
        } else {
            isnt($stderr, '', 'trigger deferred expected stderr');
            isnt($stdout, undef, 'trigger deferred expected stdout');
        }
    }
}

test_trigdeferred();
