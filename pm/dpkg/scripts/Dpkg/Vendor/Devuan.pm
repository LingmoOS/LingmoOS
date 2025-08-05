# Copyright © 2022 Guillem Jover <guillem@debian.org>
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

Dpkg::Vendor::Devuan - Devuan vendor class

=head1 DESCRIPTION

This vendor class customizes the behavior of dpkg scripts for Devuan
specific behavior and policies.

B<Note>: This is a private module, its API can change at any time.

=cut

package Dpkg::Vendor::Devuan 0.01;

use strict;
use warnings;

use parent qw(Dpkg::Vendor::Debian);

sub run_hook {
    my ($self, $hook, @params) = @_;

    if ($hook eq 'package-keyrings') {
        return ('/usr/share/keyrings/devuan-keyring.gpg',
                '/usr/share/keyrings/devuan-maintainers.gpg');
    } elsif ($hook eq 'archive-keyrings') {
        return ('/usr/share/keyrings/devuan-archive-keyring.gpg');
    } elsif ($hook eq 'archive-keyrings-historic') {
        return ('/usr/share/keyrings/devuan-archive-removed-keys.gpg');
    } elsif ($hook eq 'extend-patch-header') {
        my ($textref, $ch_info) = @params;
        if ($ch_info->{'Closes'}) {
            foreach my $bug (split(/\s+/, $ch_info->{'Closes'})) {
                $$textref .= "Bug-Devuan: https://bugs.devuan.org/$bug\n";
            }
        }
    } else {
        return $self->SUPER::run_hook($hook, @params);
    }
}

=head1 CHANGES

=head2 Version 0.xx

This is a private module.

=cut

1;
