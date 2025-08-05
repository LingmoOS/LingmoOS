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

use Test::More tests => 21;

BEGIN {
    use_ok('Dpkg::OpenPGP::KeyHandle');
}

my @ref_keys = (
    {
        type => 'auto',
        handle => '0x12345678',
        exp_type => 'keyid',
        exp_handle => '12345678',
    }, {
        type => 'auto',
        handle => '0x1234567890abcdef',
        exp_type => 'keyid',
        exp_handle => '1234567890abcdef',
    }, {
        type => 'auto',
        handle => '0x1234567890abcdef1234567890abcdef',
        exp_type => 'keyid',
        exp_handle => '1234567890abcdef1234567890abcdef',
    }, {
        type => 'auto',
        handle => 'Alice Auster',
        exp_type => 'userid',
        exp_handle => 'Alice Auster',
    }, {
        type => 'auto',
        handle => 'Alice Auster <alice@example.org>',
        exp_type => 'userid',
        exp_handle => 'Alice Auster <alice@example.org>',
    }, {
        type => 'keyid',
        handle => '0x12345678',
        exp_type => 'keyid',
        exp_handle => '12345678',
    }, {
        type => 'keyid',
        handle => '0x1234567890abcdef',
        exp_type => 'keyid',
        exp_handle => '1234567890abcdef',
    }, {
        type => 'keyid',
        handle => '0x1234567890abcdef1234567890abcdef',
        exp_type => 'keyid',
        exp_handle => '1234567890abcdef1234567890abcdef',
    }, {
        type => 'userid',
        handle => 'Alice Auster',
        exp_type => 'userid',
        exp_handle => 'Alice Auster',
    }, {
        type => 'userid',
        handle => 'Alice Auster <alice@example.org>',
        exp_type => 'userid',
        exp_handle => 'Alice Auster <alice@example.org>',
    }
);

foreach my $ref_key (@ref_keys) {
    my $key = Dpkg::OpenPGP::KeyHandle->new(
        type => $ref_key->{type},
        handle => $ref_key->{handle},
    );
    is($key->type, $ref_key->{exp_type},
        'key type ' . $key->type . " sanitized as $ref_key->{exp_type}");
    is($key->handle, $ref_key->{exp_handle},
        'key handle ' . $key->handle . " sanitized as $ref_key->{exp_handle}");
}

# TODO: Add actual test cases.
