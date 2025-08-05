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

use Test::More tests => 10;
use Test::Dpkg qw(:paths);

use File::Compare;
use File::Path qw(rmtree);

BEGIN {
    use_ok('Dpkg::File');
}

my $datadir = test_get_data_path();
my $tempdir = test_get_temp_path();

my ($data, $data_ref, $data_fh);

$data = file_slurp("$datadir/slurp-me");
$data_ref = <<'DATA';
first line
next line
final line
DATA
is($data, $data_ref, 'slurped data');

file_dump("$tempdir/slurp-me", $data);
ok(compare("$tempdir/slurp-me", "$datadir/slurp-me") == 0,
    'dumped slurped data');

open $data_fh, '<', "$datadir/slurp-me"
    or die "cannot open $datadir/slurp-me for reading: $!";
my $discard = <$data_fh>;
$data = file_slurp($data_fh);
close $data_fh;
$data_ref = <<'DATA';
next line
final line
DATA
is($data, $data_ref, 'slurped partial data');

file_dump("$tempdir/dump-partial", $data);
ok(compare("$tempdir/dump-partial", "$datadir/dump-partial") == 0,
    'dumped slurped partial data');

open $data_fh, '>', "$tempdir/append-me"
    or die "cannot create $tempdir/append-me: $!";
print { $data_fh } "append line\n";
file_dump($data_fh, "new line\nend line\n");
close $data_fh;
ok(compare("$tempdir/append-me", "$datadir/append-me") == 0,
    'dumped appended data');

$data = undef;
eval {
    $data = file_slurp("$datadir/non-existent");
};
ok($@, 'cannot slurp missing file');

ok(! -e "$tempdir/touched", 'file to be touched does not exist');
file_touch("$tempdir/touched");
ok(-e "$tempdir/touched", 'touched file exists');
ok(-z "$tempdir/touched", 'touched file is empty');
