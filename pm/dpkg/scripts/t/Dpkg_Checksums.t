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

use Test::More tests => 59;
use Test::Dpkg qw(:paths);

BEGIN {
    use_ok('Dpkg::Checksums');
}

my $datadir = test_get_data_path();

my @data = (
    {
        file => 'empty',
        size => 0,
        sums => {
            md5 => 'd41d8cd98f00b204e9800998ecf8427e',
            sha1 => 'da39a3ee5e6b4b0d3255bfef95601890afd80709',
            sha256 => 'e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855',
        }
    }, {
        file => 'data-1',
        size => 7,
        sums => {
            md5 => '1b662eff496fde1a63cc5ff97beec10a',
            sha1 => 'ff66a3dc152f273a19392d3099b2915c311c707e',
            sha256 => 'f53cb4ee5128363210053c89627757c3dd864ab87e3ac9bff20dd6fe4175a140',
        }
    }, {
        file => 'data-2',
        size => 14,
        sums => {
            md5 => '785400cfc51d16a06e2c34aa511b99ef',
            sha1 => '329ba56c0c9c63b6e138f3970ac3628e476a6854',
            sha256 => '217147cd3126a076ba3fd816566a80aaaffff5facc572394dbd6af61a49760d1',
        }
    }
);

my %str_checksum;
foreach my $f (@data) {
    foreach my $alg (keys %{$f->{sums}}) {
        $str_checksum{$alg} .= "\n$f->{sums}->{$alg} $f->{size} $f->{file}";
    }
}

sub test_checksums {
    my $ck = shift;

    my @known_files = $ck->get_files();
    my @data_files = map { $_->{file} } @data;

    is_deeply(\@known_files, \@data_files, 'List of files');
    foreach my $f (@data) {
        ok($ck->has_file($f->{file}), "Known file $f->{file}");
        is($ck->get_size($f->{file}), $f->{size}, "Known file $f->{file} size");
        is_deeply($ck->get_checksum($f->{file}), $f->{sums},
                  "Known file $f->{file} checksums");
    }
}


my @expected_checksums = qw(md5 sha1 sha256);
my @known_checksums = checksums_get_list();

is_deeply(\@known_checksums, \@expected_checksums, 'List of known checksums');

foreach my $c (@expected_checksums) {
    ok(checksums_is_supported($c), "Checksum $c is supported");

    my $uc = uc $c;
    ok(checksums_is_supported($uc), "Checksum $uc (uppercase) is supported");

    ok(defined checksums_get_property($c, 'name'), "Checksum $c has name");
    ok(defined checksums_get_property($c, 'regex'), "Checksum $c has regex");
    ok(defined checksums_get_property($c, 'strong'), "Checksum $c has strong");
}

my $ck = Dpkg::Checksums->new();

is(scalar $ck->get_files(), 0, 'No checksums recorded');

# Check add_from_file()

foreach my $f (@data) {
    $ck->add_from_file("$datadir/$f->{file}", key => $f->{file});
}

foreach my $alg (keys %str_checksum) {
    my $str = $ck->export_to_string($alg);
    is($str, $str_checksum{$alg}, "Export checksum $alg to string from file");
}

test_checksums($ck);

# Check add_from_string()

foreach my $alg (keys %str_checksum) {
    $ck->add_from_string($alg, $str_checksum{$alg});

    my $str = $ck->export_to_string($alg);
    is($str, $str_checksum{$alg}, "Export checksum $alg to string from string");
}

test_checksums($ck);

# Check remove_file()

ok($ck->has_file('data-2'), 'To be removed file is present');
$ck->remove_file('data-2');
ok(!$ck->has_file('data-2'), 'Remove file is not present');

# Check add_from_control()
my $ctrl;
foreach my $f (@data) {
    next if $f->{file} ne 'data-2';
    foreach my $alg (keys %{$f->{sums}}) {
        $ctrl->{"Checksums-$alg"} = "\n$f->{sums}->{$alg} $f->{size} $f->{file}";
    }
}
$ck->add_from_control($ctrl);

test_checksums($ck);

# Check export_to_control()

my $ctrl_export = {};
$ck->export_to_control($ctrl_export);

foreach my $alg (keys %str_checksum) {
    is($ctrl_export->{"Checksums-$alg"}, $str_checksum{$alg},
       "Export checksum $alg to a control object");
}
