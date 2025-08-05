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

use List::Util qw(any);
use File::Find;
use Module::Metadata;

use Test::More;
use Test::Dpkg qw(:needs);

test_needs_author();
test_needs_module('Test::Pod::Coverage');
test_needs_srcdir_switch();

sub all_pod_modules
{
    my @modules_todo = @_;
    my @modules;
    my $scan_perl_modules = sub {
        my $module = $File::Find::name;

        # Only check modules, scripts are documented in man pages.
        return unless $module =~ s/\.pm$//;

        my $mod = Module::Metadata->new_from_file($File::Find::name);

        # As a first step just check public modules (version > 0.xx).
        return if $mod->version() =~ m/^0\.\d\d$/;

        $module =~ s{^\Q$File::Find::topdir\E/}{};
        $module =~ s{/}{::}g;

        return if any { $module eq $_ } @modules_todo;

        push @modules, $module;
    };

    my %options = (
        wanted => $scan_perl_modules,
        no_chdir => 1,
    );
    find(\%options, Test::Dpkg::test_get_perl_dirs());

    return @modules;
}

my @modules_todo = qw(Dpkg::Arch Dpkg::Source::Package);
my @modules = all_pod_modules(@modules_todo);

plan tests => scalar @modules + scalar @modules_todo;

for my $module (@modules) {
    pod_coverage_ok($module);
}

TODO: {
    local $TODO = 'modules partially documented';

    for my $module (@modules_todo) {
        pod_coverage_ok($module);
    }
}
