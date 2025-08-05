# Copyright © 2009-2011 Raphaël Hertzog <hertzog@debian.org>
# Copyright © 2009-2024 Guillem Jover <guillem@debian.org>
# Copyright © 2017-2019 Matthias Klumpp <matthias.klumpp@puri.sm>
# Copyright © 2021 Jonas Smedegaard <dr@jones.dk>
# Copyright © 2021 Purism, SPC
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

Dpkg::Vendor::PureOS - PureOS vendor class

=head1 DESCRIPTION

This vendor class customizes the behavior of dpkg scripts for PureOS
specific behavior and policies.

B<Note>: This is a private module, its API can change at any time.

=cut

package Dpkg::Vendor::PureOS 0.01;

use strict;
use warnings;

use Dpkg::ErrorHandling;
use Dpkg::Gettext;
use Dpkg::Control::Types;

use parent qw(Dpkg::Vendor::Debian);

sub run_hook {
    my ($self, $hook, @params) = @_;

    if ($hook eq 'before-source-build') {
        my $src = shift @params;
        my $fields = $src->{fields};

        if (defined($fields->{'Version'}) and defined($fields->{'Maintainer'}) and
           $fields->{'Version'} =~ /pureos/) {
               unless ($fields->{'Original-Maintainer'}) {
                   warning(g_('Version number suggests PureOS changes, but there is no XSBC-Original-Maintainer field'));
               }
        }

    } elsif ($hook eq 'keyrings') {
        return $self->run_hook('package-keyrings', @params);
    } elsif ($hook eq 'package-keyrings') {
        return ($self->SUPER::run_hook($hook),
                '/usr/share/keyrings/pureos-archive-keyring.gpg');
    } elsif ($hook eq 'archive-keyrings') {
        return ($self->SUPER::run_hook($hook),
                '/usr/share/keyrings/pureos-archive-keyring.gpg');
    } elsif ($hook eq 'archive-keyrings-historic') {
        return ($self->SUPER::run_hook($hook),
                '/usr/share/keyrings/pureos-archive-removed-keys.gpg');
    } else {
        return $self->SUPER::run_hook($hook, @params);
    }
}

=head1 CHANGES

=head2 Version 0.xx

This is a private module.

=cut

1;
