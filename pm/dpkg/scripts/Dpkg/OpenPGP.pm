# Copyright © 2017 Guillem Jover <guillem@debian.org>
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

Dpkg::OpenPGP - multi-backend OpenPGP support

=head1 DESCRIPTION

This module provides a class for transparent multi-backend OpenPGP support.

B<Note>: This is a private module, its API can change at any time.

=cut

package Dpkg::OpenPGP 0.01;

use strict;
use warnings;

use List::Util qw(none);

use Dpkg::Gettext;
use Dpkg::ErrorHandling;
use Dpkg::IPC;
use Dpkg::Path qw(find_command);

my @BACKENDS = qw(
    sop
    sq
    gpg
);
my %BACKEND = (
    sop => 'SOP',
    sq => 'Sequoia',
    gpg => 'GnuPG',
);

sub new {
    my ($this, %opts) = @_;
    my $class = ref($this) || $this;

    my $self = {};
    bless $self, $class;

    my $backend = $opts{backend} // 'auto';
    my %backend_opts = (
        cmdv => $opts{cmdv} // 'auto',
        cmd => $opts{cmd} // 'auto',
    );

    if ($backend eq 'auto') {
        # Defaults for stateless full API auto-detection.
        $opts{needs}{api} //= 'full';
        $opts{needs}{keystore} //= 0;

        if (none { $opts{needs}{api} eq $_ } qw(full verify)) {
            error(g_('unknown OpenPGP api requested %s'), $opts{needs}{api});
        }

        $self->{backend} = $self->_auto_backend($opts{needs}, %backend_opts);
    } elsif (exists $BACKEND{$backend}) {
        $self->{backend} = $self->_load_backend($BACKEND{$backend}, %backend_opts);
        if (! $self->{backend}) {
            error(g_('cannot load OpenPGP backend %s'), $backend);
        }
    } else {
        error(g_('unknown OpenPGP backend %s'), $backend);
    }

    return $self;
}

sub _load_backend {
    my ($self, $backend, %opts) = @_;

    my $module = "Dpkg::OpenPGP::Backend::$backend";
    eval qq{
        require $module;
    };
    return if $@;

    return $module->new(%opts);
}

sub _auto_backend {
    my ($self, $needs, %opts) = @_;

    foreach my $backend (@BACKENDS) {
        my $module = $self->_load_backend($BACKEND{$backend}, %opts);

        if ($needs->{api} eq 'verify') {
            next if ! $module->has_verify_cmd();
        } else {
            next if ! $module->has_backend_cmd();
        }
        next if $needs->{keystore} && ! $module->has_keystore();

        return $module;
    }

    # Otherwise load a dummy backend.
    return Dpkg::OpenPGP::Backend->new();
}

sub can_use_secrets {
    my ($self, $key) = @_;

    return 0 unless $self->{backend}->has_backend_cmd();
    return 0 if $key->type eq 'keyfile' && ! -f $key->handle;
    return 0 if $key->type eq 'keystore' && ! -e $key->handle;
    return 0 unless $self->{backend}->can_use_key($key);
    return 1;
}

sub get_trusted_keyrings {
    my $self = shift;

    return $self->{backend}->get_trusted_keyrings();
}

sub armor {
    my ($self, $type, $in, $out) = @_;

    return $self->{backend}->armor($type, $in, $out);
}

sub dearmor {
    my ($self, $type, $in, $out) = @_;

    return $self->{backend}->dearmor($type, $in, $out);
}

sub inline_verify {
    my ($self, $inlinesigned, $data, @certs) = @_;

    return $self->{backend}->inline_verify($inlinesigned, $data, @certs);
}

sub verify {
    my ($self, $data, $sig, @certs) = @_;

    return $self->{backend}->verify($data, $sig, @certs);
}

sub inline_sign {
    my ($self, $data, $inlinesigned, $key) = @_;

    return $self->{backend}->inline_sign($data, $inlinesigned, $key);
}

=head1 CHANGES

=head2 Version 0.xx

This is a private module.

=cut

1;
