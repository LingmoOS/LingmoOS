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

use Test::More;
use Test::Dpkg qw(:paths :needs);

use File::Compare;

use Dpkg::ErrorHandling;
use Dpkg::Path qw(find_command);
use Dpkg::OpenPGP::KeyHandle;

my @backends = test_needs_openpgp_backend();
unshift @backends, {
    backend => 'auto',
    cmd => 'auto',
    cmdv => 'auto',
};

plan tests => 2 + 15 * scalar @backends;

use_ok('Dpkg::OpenPGP');
use_ok('Dpkg::OpenPGP::ErrorCodes');

report_options(quiet_warnings => 1);

sub test_diff
{
    my ($exp_file, $gen_file, $desc) = @_;

    my $res = compare($exp_file, $gen_file);
    if ($res) {
        system "diff -u '$exp_file' '$gen_file' >&2";
    }
    ok($res == 0, "$desc ($exp_file vs $gen_file)");
}

foreach my $backend_opts (@backends) {
    my $datadir = test_get_data_path();
    my $tempdir = test_get_temp_path();

    my $backend = $backend_opts->{backend};
    my $cmd = $backend_opts->{cmd} || 'none';
    my $cmdv = $backend_opts->{cmdv} || $cmd;
    if ($cmd ne 'none' && $cmdv eq 'none') {
        $cmdv = $cmd;
    }
    my $openpgp = Dpkg::OpenPGP->new(%{$backend_opts});

    my $certfile = "$datadir/dpkg-test-pub.asc";
    my $keyfile  = "$datadir/dpkg-test-sec.asc";

    note("openpgp backend=$backend cmd=$cmd cmdv=$cmdv");

    SKIP: {
        skip 'missing backend command', 9
            unless $openpgp->{backend}->has_backend_cmd();

        ok($openpgp->dearmor('PUBLIC KEY BLOCK', $certfile, "$tempdir/dpkg-test-pub.pgp") == OPENPGP_OK(),
            "($backend:$cmd) dearmoring OpenPGP ASCII Armored certificate");
        ok($openpgp->armor('PUBLIC KEY BLOCK', "$tempdir/dpkg-test-pub.pgp", "$tempdir/dpkg-test-pub.asc") == OPENPGP_OK(),
            "($backend:$cmd) armoring OpenPGP binary certificate");
        test_diff($certfile, "$tempdir/dpkg-test-pub.asc",
            "($backend:$cmd) OpenPGP certificate dearmor/armor round-trip correctly");

        ok($openpgp->armor('SIGNATURE', "$datadir/sign-file.sig", "$tempdir/sign-file.asc") == OPENPGP_OK(),
            "($backend:$cmd) armoring OpenPGP binary signature succeeded");
        ok(compare("$datadir/sign-file.sig", "$tempdir/sign-file.asc") != 0,
            "($backend:$cmd) armoring OpenPGP ASCII Armor changed the file");
        ok($openpgp->armor('SIGNATURE', "$datadir/sign-file.asc", "$tempdir/sign-file-rearmor.asc") == OPENPGP_OK(),
            "($backend:$cmd) armoring OpenPGP armored signature succeeded");
        test_diff("$datadir/sign-file.asc", "$tempdir/sign-file-rearmor.asc",
            "($backend:$cmd) rearmoring OpenPGP ASCII Armor changed the file");

        ok($openpgp->dearmor('SIGNATURE', "$tempdir/sign-file.asc", "$tempdir/sign-file.sig") == OPENPGP_OK(),
            "($backend:$cmd) dearmoring OpenPGP armored signature succeeded");
        test_diff("$datadir/sign-file.sig", "$tempdir/sign-file.sig",
            "($backend:$cmd) dearmored OpenPGP ASCII Armor signature matches");
    };

    SKIP: {
        skip 'missing verify command', 4
            unless $openpgp->{backend}->has_verify_cmd();

        ok($openpgp->inline_verify("$datadir/sign-file-inline.asc", undef, $certfile) == OPENPGP_OK(),
            "($backend:$cmdv) verify OpenPGP ASCII Armor inline signature");
        ok($openpgp->inline_verify("$datadir/sign-file-inline.sig", undef, $certfile) == OPENPGP_OK(),
            "($backend:$cmdv) verify OpenPGP binary inline signature");

        ok($openpgp->verify("$datadir/sign-file", "$datadir/sign-file.asc", $certfile) == OPENPGP_OK(),
            "($backend:$cmdv) verify OpenPGP ASCII Armor detached signature");
        ok($openpgp->verify("$datadir/sign-file", "$datadir/sign-file.sig", $certfile) == OPENPGP_OK(),
            "($backend:$cmdv) verify OpenPGP binary detached signature");
    };

    my $key = Dpkg::OpenPGP::KeyHandle->new(
        type => 'keyfile',
        handle => $keyfile,
    );

    SKIP: {
        skip 'cannot use secrets', 2 unless $openpgp->can_use_secrets($key);

        ok($openpgp->inline_sign("$datadir/sign-file", "$tempdir/sign-file-inline.asc", $key) == OPENPGP_OK(),
            "($backend:$cmd) inline OpenPGP sign");
        ok($openpgp->inline_verify("$tempdir/sign-file-inline.asc", undef, $certfile) == OPENPGP_OK(),
            "($backend:$cmd) verify generated inline OpenPGP signature");
    };

    # TODO: Add more test cases.
}
