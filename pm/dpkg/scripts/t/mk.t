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

use Test::More tests => 12;
use Test::Dpkg qw(:paths);

use File::Spec::Functions qw(rel2abs);

use Dpkg ();
use Dpkg::ErrorHandling;
use Dpkg::IPC;
use Dpkg::Vendor;

my $srcdir = rel2abs($ENV{srcdir} || '.');
my $datadir = test_get_data_path();

# Turn these into absolute names so that we can safely switch to the test
# directory with «make -C».
$ENV{$_} = rel2abs($ENV{$_}) foreach qw(srcdir DPKG_DATADIR DPKG_ORIGINS_DIR);

# Any parallelization from the parent should be ignored, we are testing
# the makefiles serially anyway.
delete $ENV{MAKEFLAGS};

# Delete other variables that can affect the tests.
delete $ENV{$_} foreach grep { m/^DEB_/ } keys %ENV;

# Set architecture variables to not require dpkg nor gcc.
$ENV{CC} = 'gcc';
$ENV{PATH} = "$srcdir/t/mock-bin:$ENV{PATH}";
$ENV{DEB_BUILD_PATH} = rel2abs($datadir);

sub test_makefile {
    my ($makefile, $desc) = @_;

    $desc //= 'default';

    spawn(exec => [ $Dpkg::PROGMAKE, '-C', $datadir, '-f', $makefile ],
          wait_child => 1, nocheck => 1);
    ok($? == 0, "makefile $makefile computes all values correctly ($desc)");
}

sub cmd_get_vars {
    my (@cmd) = @_;
    my %var;

    open my $cmd_fh, '-|', @cmd or subprocerr($cmd[0]);
    while (<$cmd_fh>) {
        chomp;
        my ($key, $value) = split /=/, $_, 2;
        $var{$key} = $value;
    }
    close $cmd_fh or subprocerr($cmd[0]);

    return %var;
}

# Test makefiles.

my %arch = cmd_get_vars($ENV{PERL}, "$srcdir/dpkg-architecture.pl", '-f');

while (my ($k, $v) = each %arch) {
    delete $ENV{$k};
    $ENV{"TEST_$k"} = $v;
}
test_makefile('architecture.mk', 'without envvars');
while (my ($k, $v) = each %arch) {
    $ENV{$k} = $v;
}
test_makefile('architecture.mk', 'with envvars');

test_makefile('buildapi.mk');

$ENV{DEB_BUILD_OPTIONS} = 'parallel=16';
$ENV{TEST_DEB_BUILD_OPTION_PARALLEL} = '16';
test_makefile('buildopts.mk');
delete $ENV{DEB_BUILD_OPTIONS};
delete $ENV{TEST_DEB_BUILD_OPTION_PARALLEL};

my %buildflag = cmd_get_vars($ENV{PERL}, "$srcdir/dpkg-buildflags.pl");

while (my ($var, $flags) = each %buildflag) {
    delete $ENV{$var};
    $ENV{"TEST_$var"} = $flags;
}
test_makefile('buildflags.mk');

my %buildtools = (
    AS => 'as',
    CPP => 'gcc -E',
    CC => 'gcc',
    CXX => 'g++',
    OBJC => 'gcc',
    OBJCXX => 'g++',
    F77 => 'gfortran',
    FC => 'gfortran',
    LD => 'ld',
    STRIP => 'strip',
    OBJCOPY => 'objcopy',
    OBJDUMP => 'objdump',
    NM => 'nm',
    AR => 'ar',
    RANLIB => 'ranlib',
    PKG_CONFIG => 'pkgconf',
);

while (my ($var, $tool) = each %buildtools) {
    delete $ENV{$var};
    $ENV{"TEST_$var"} = "$ENV{DEB_HOST_GNU_TYPE}-$tool";
    delete $ENV{"${var}_FOR_BUILD"};
    $ENV{"TEST_${var}_FOR_BUILD"} = "$ENV{DEB_BUILD_GNU_TYPE}-$tool";
}
test_makefile('buildtools.mk', 'without envvars');

$ENV{DEB_BUILD_OPTIONS} = 'nostrip';
$ENV{TEST_STRIP} = ':';
$ENV{TEST_STRIP_FOR_BUILD} = ':';
test_makefile('buildtools.mk', 'with envvars');
delete $ENV{DEB_BUILD_OPTIONS};

foreach my $tool (keys %buildtools) {
    delete $ENV{${tool}};
    delete $ENV{"${tool}_FOR_BUILD"};
}

delete $ENV{SOURCE_DATE_EPOCH};
# Timestamp in seconds since the epoch from date in test debian/changelog
# entry: «Tue, 04 Aug 2015 16:13:50 +0200».
$ENV{TEST_SOURCE_DATE_EPOCH} = 1438697630;
test_makefile('pkg-info.mk');

$ENV{SOURCE_DATE_EPOCH} = 100;
$ENV{TEST_SOURCE_DATE_EPOCH} = 100;
test_makefile('pkg-info.mk');

test_makefile('vendor.mk');
test_makefile('vendor-v0.mk');
test_makefile('vendor-v1.mk');
