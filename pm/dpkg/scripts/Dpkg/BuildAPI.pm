# Copyright © 2020-2022 Guillem Jover <guillem@debian.org>
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

=encoding utf8

=head1 NAME

Dpkg::BuildAPI - handle build API versions

=head1 DESCRIPTION

The Dpkg::BuildAPI module provides functions to fetch the current dpkg
build API level.

B<Note>: This is a private module, its API can change at any time.

=cut

package Dpkg::BuildAPI 0.01;

use strict;
use warnings;

our @EXPORT_OK = qw(
    get_build_api
    reset_build_api
);

use Exporter qw(import);

use Dpkg::Gettext;
use Dpkg::ErrorHandling;
use Dpkg::BuildEnv;
use Dpkg::Version;
use Dpkg::Deps;

use constant {
    DEFAULT_BUILD_API => '0',
    MAX_BUILD_API => '1',
};

my $build_api;

=head1 FUNCTIONS

=over 4

=item $level = get_build_api([$ctrl])

Get the build API level, from the environment variable B<DPKG_BUILD_API>,
or if not defined and a $ctrl L<Dpkg::Control::Info> object passed as an
argument, from its build dependency fields. If no $ctrl object gets passed
the previous value obtained is returned.

=cut

sub get_build_api {
    my $ctrl = shift;

    return $build_api if defined $build_api && ! defined $ctrl;

    if (Dpkg::BuildEnv::has('DPKG_BUILD_API')) {
        $build_api = Dpkg::BuildEnv::get('DPKG_BUILD_API');
    } elsif (defined $ctrl) {
        my $src = $ctrl->get_source();
        my @dep_list = deps_concat(map {
            $src->{$_ }
        } qw(Build-Depends Build-Depends-Indep Build-Depends-Arch));

        my $deps = deps_parse(@dep_list,
            build_dep => 1,
            reduce_restrictions => 1,
        );

        if (not defined $deps) {
            $build_api = DEFAULT_BUILD_API;
            return $build_api;
        }

        deps_iterate($deps, sub {
            my $dep = shift;

            return 1 if $dep->{package} ne 'dpkg-build-api';

            if (! defined $dep->{relation} || $dep->{relation} ne REL_EQ) {
                error(g_('dpkg build API level needs an exact version'));
            }

            if (defined $build_api and $build_api ne $dep->{version}) {
                error(g_('dpkg build API level with conflicting versions: %s vs %s'),
                      $build_api, $dep->{version});
            }

            $build_api = $dep->{version};

            return 1;
        });
    }

    $build_api //= DEFAULT_BUILD_API;

    if ($build_api !~ m/^[0-9]+$/) {
        error(g_("invalid dpkg build API level '%s'"), $build_api);
    }

    if ($build_api > MAX_BUILD_API) {
        error(g_("dpkg build API level '%s' greater than max '%s'"),
              $build_api, MAX_BUILD_API);
    }

    return $build_api;
}

=item reset_build_api()

Reset the cached build API level.

=cut

sub reset_build_api {
    $build_api = undef;
}

=back

=head1 CHANGES

=head2 Version 0.xx

This is a private module.

=cut

1;
