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

use Test::More tests => 19;

BEGIN {
    use_ok('Dpkg::BuildFlags');
}

sub test_optflag
{
    my ($bf, $optflag) = @_;

    foreach my $flag (qw(CFLAGS CXXFLAGS OBJCFLAGS OBJCXXFLAGS
                         FFLAGS FCFLAGS)) {
        my $value = $bf->get($flag);
        ok($value =~ m/$optflag/, "$flag contains $optflag: $value");
    }
}

sub test_ltoflag
{
    my $bf = shift;

    # Test the LTO flags enabled by default on some arches.
    ok($bf->get('LDFLAGS') =~ m/-flto=auto -ffat-lto-objects/,
        "LDFLAGS contains LTO flags on $ENV{DEB_HOST_ARCH}");
}

sub test_no_ltoflag
{
    my $bf = shift;

    # Test the LTO flags not being enabled.
    ok($bf->get('LDFLAGS') !~ m/-flto=auto -ffat-lto-objects/,
        "LDFLAGS does not contain LTO flags on $ENV{DEB_HOST_ARCH}");
}

my $bf;

undef $ENV{DEB_BUILD_MAINT_OPTIONS};

# Force loading the Dpkg::Vendor::Ubuntu module.
$ENV{DEB_VENDOR} = 'Ubuntu';

# Test the optimization flag inherited from the Dpkg::Vendor::Debian module.
$ENV{DEB_HOST_ARCH} = 'amd64';
$bf = Dpkg::BuildFlags->new();

test_optflag($bf, '-O2');
test_ltoflag($bf);

# Test the overlaid Ubuntu-specific linker flag.
ok($bf->get('LDFLAGS') =~ m/-Wl,-Bsymbolic-functions/,
   'LDFLAGS contains -Bsymbolic-functions');

# Test the optimization flag override only for ppc64el.
$ENV{DEB_HOST_ARCH} = 'ppc64el';
$bf = Dpkg::BuildFlags->new();

test_optflag($bf, '-O3');
test_ltoflag($bf);

# Test the optimization flag not enabled for riscv64.
$ENV{DEB_HOST_ARCH} = 'riscv64';
$bf = Dpkg::BuildFlags->new();

test_no_ltoflag($bf);

# Test the optimization flag override by DEB_BUILD_MAINT_OPTIONS.
$ENV{DEB_BUILD_MAINT_OPTIONS} = 'optimize=+lto';
$bf = Dpkg::BuildFlags->new();

test_ltoflag($bf);

$ENV{DEB_HOST_ARCH} = 'amd64';
$ENV{DEB_BUILD_MAINT_OPTIONS} = 'optimize=-lto';
$bf = Dpkg::BuildFlags->new();
test_no_ltoflag($bf);
