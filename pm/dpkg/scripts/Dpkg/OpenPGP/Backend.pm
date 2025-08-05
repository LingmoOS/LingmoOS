# Copyright © 2017, 2022 Guillem Jover <guillem@debian.org>
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

Dpkg::OpenPGP::Backend - OpenPGP backend base class

=head1 DESCRIPTION

This module provides an OpenPGP backend base class that specific
implementations should inherit from.

B<Note>: This is a private module, its API can change at any time.

=cut

package Dpkg::OpenPGP::Backend 0.01;

use strict;
use warnings;

use List::Util qw(first);
use MIME::Base64;

use Dpkg::ErrorHandling;
use Dpkg::Gettext;
use Dpkg::Path qw(find_command);
use Dpkg::File;
use Dpkg::OpenPGP::ErrorCodes;

sub DEFAULT_CMDV {
    return [];
}

sub DEFAULT_CMDSTORE {
    return [];
}

sub DEFAULT_CMD {
    return [];
}

sub _detect_cmd {
    my ($cmd, $default) = @_;

    if (! defined $cmd || $cmd eq 'auto') {
        return first { find_command($_) } @{$default};
    } elsif ($cmd eq 'none') {
        return;
    } else {
        return find_command($cmd);
    }
}

sub new {
    my ($this, %opts) = @_;
    my $class = ref($this) || $this;

    my $self = {};
    bless $self, $class;

    $self->{cmdv} = _detect_cmd($opts{cmdv}, $self->DEFAULT_CMDV());
    $self->{cmdstore} = _detect_cmd($opts{cmdstore}, $self->DEFAULT_CMDSTORE());
    $self->{cmd} = _detect_cmd($opts{cmd}, $self->DEFAULT_CMD());

    return $self;
}

sub has_backend_cmd {
    my $self = shift;

    return defined $self->{cmd};
}

sub has_verify_cmd {
    my $self = shift;

    return 1 if @{$self->DEFAULT_CMDV()} && defined $self->{cmdv};
    return 1 if defined $self->{cmd};
    return 0;
}

sub has_keystore {
    my $self = shift;

    return 0;
}

sub can_use_key {
    my ($self, $key) = @_;

    return $self->has_keystore() if $key->needs_keystore();
    return 1;
}

sub get_trusted_keyrings {
    my $self = shift;

    return ();
}

# _pgp_* functions are strictly for applying or removing ASCII armor.
# See <https://www.rfc-editor.org/rfc/rfc9580.html#section-6> for more
# details.

sub _pgp_dearmor_data {
    my ($type, $data, $filename) = @_;

    # Note that we ignore an incorrect or absent checksum, following the
    # guidance of <https://www.rfc-editor.org/rfc/rfc9580.html>.
    my $armor_regex = qr{
        -----BEGIN\ PGP\ \Q$type\E-----[\r\t ]*\n
        (?:[^:\n]+:\ [^\n]*[\r\t ]*\n)*
        [\r\t ]*\n
        ([a-zA-Z0-9/+\n]+={0,2})[\r\t ]*\n
        (?:=[a-zA-Z0-9/+]{4}[\r\t ]*\n)?
        -----END\ PGP\ \Q$type\E-----
    }xm;

    my $blocks = 0;
    my $binary;
    while ($data =~ m/$armor_regex/g) {
        $binary .= decode_base64($1);
        $blocks++;
    }
    if ($blocks > 1) {
        warning(g_('multiple concatenated ASCII Armor blocks in %s, ' .
                   'which is not an interoperable construct, see <%s>'),
                $filename,
                'https://tests.sequoia-pgp.org/results.html#ASCII_Armor');
        hint(g_('sq keyring merge --overwrite --output %s %s'),
             $filename, $filename);
    }
    return $binary;
}

sub _pgp_armor_checksum {
    my ($data) = @_;

    # From RFC9580 <https://www.rfc-editor.org/rfc/rfc9580.html>.
    #
    # The resulting three-octet-wide value then gets base64-encoded into
    # four base64 ASCII characters.

    ## no critic (ValuesAndExpressions::ProhibitMagicNumbers)
    my $CRC24_INIT = 0xB704CE;
    my $CRC24_GENERATOR = 0x864CFB;

    my @bytes = unpack 'C*', $data;
    my $crc = $CRC24_INIT;
    for my $b (@bytes) {
        $crc ^= ($b << 16);
        for (1 .. 8) {
            $crc <<= 1;
            if ($crc & 0x1000000) {
                # Clear bit 25 to avoid overflow.
                $crc &= 0xffffff;
                $crc ^= $CRC24_GENERATOR;
            }
        }
    }
    my $sum = pack 'CCC', ($crc >> 16) & 0xff, ($crc >> 8) & 0xff, $crc & 0xff;
    return encode_base64($sum, q{});
}

sub _pgp_armor_data {
    my ($type, $data) = @_;

    my $out = encode_base64($data, q{}) =~ s/(.{1,64})/$1\n/gr;
    chomp $out;
    my $crc = _pgp_armor_checksum($data);
    my $armor = <<~"ARMOR";
        -----BEGIN PGP $type-----

        $out
        =$crc
        -----END PGP $type-----
        ARMOR
    return $armor;
}

sub armor {
    my ($self, $type, $in, $out) = @_;

    my $raw_data = file_slurp($in);
    my $data = _pgp_dearmor_data($type, $raw_data, $in) // $raw_data;
    my $armor = _pgp_armor_data($type, $data);
    return OPENPGP_BAD_DATA unless defined $armor;
    file_dump($out, $armor);

    return OPENPGP_OK;
}

sub dearmor {
    my ($self, $type, $in, $out) = @_;

    my $armor = file_slurp($in);
    my $data = _pgp_dearmor_data($type, $armor, $in);
    return OPENPGP_BAD_DATA unless defined $data;
    file_dump($out, $data);

    return OPENPGP_OK;
}

sub inline_verify {
    my ($self, $inlinesigned, $data, @certs) = @_;

    return OPENPGP_UNSUPPORTED_SUBCMD;
}

sub verify {
    my ($self, $data, $sig, @certs) = @_;

    return OPENPGP_UNSUPPORTED_SUBCMD;
}

sub inline_sign {
    my ($self, $data, $inlinesigned, $key) = @_;

    return OPENPGP_UNSUPPORTED_SUBCMD;
}

=head1 CHANGES

=head2 Version 0.xx

This is a private module.

=cut

1;
