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

Dpkg::OpenPGP::KeyHandle - OpenPGP key handle support

=head1 DESCRIPTION

This module provides a class to hold an OpenPGP key handle, as a way for
the code to refer to a key in an independent way.

B<Note>: This is a private module, its API can change at any time.

=cut

package Dpkg::OpenPGP::KeyHandle 0.01;

use strict;
use warnings;

use Carp;
use List::Util qw(any none);

sub new {
    my ($this, %opts) = @_;
    my $class = ref($this) || $this;

    my $self = {
        type => $opts{type} // 'auto',
        handle => $opts{handle},
    };
    bless $self, $class;

    $self->_sanitize();

    return $self;
}

my $keyid_regex = qr/^(?:0x)?([[:xdigit:]]+)$/;

sub _sanitize {
    my ($self) = shift;

    my $type = $self->{type};
    if ($type eq 'auto') {
        if (-e $self->{handle}) {
            $type = 'keyfile';
        } else {
            $type = 'autoid';
        }
    }

    if ($type eq 'autoid') {
        if ($self->{handle} =~ m/$keyid_regex/) {
            $self->{handle} = $1;
            $type = 'keyid';
        } else {
            $type = 'userid';
        }
        $self->{type} = $type;
    } elsif ($type eq 'keyid') {
        if ($self->{handle} =~ m/$keyid_regex/) {
            $self->{handle} = $1;
        }
    }

    if (none { $type eq $_ } qw(userid keyid keyfile keystore)) {
        croak "unknown type parameter value $type";
    }

    return;
}

sub needs_keystore {
    my $self = shift;

    return any { $self->{type} eq $_ } qw(keyid userid);
}

sub set {
    my ($self, $type, $handle) = @_;

    $self->{type} = $type;
    $self->{handle} = $handle;

    $self->_sanitize();
}

sub type {
    my $self = shift;

    return $self->{type};
}

sub handle {
    my $self = shift;

    return $self->{handle};
}

=head1 CHANGES

=head2 Version 0.xx

This is a private module.

=cut

1;
