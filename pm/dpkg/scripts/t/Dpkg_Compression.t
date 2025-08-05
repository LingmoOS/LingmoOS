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

use Test::More tests => 48;
use Test::Dpkg qw(:paths);

use IPC::Cmd qw(can_run);

use_ok('Dpkg::Compression');
use_ok('Dpkg::Compression::FileHandle');

my $tmpdir = test_get_temp_path();
my @lines = ("One\n", "Two\n", "Three\n");
my $fh;
my $have_gunzip = can_run('gunzip');

sub test_write {
    my ($filename, $check_result) = @_;

    $fh = Dpkg::Compression::FileHandle->new();
    open $fh, '>', $filename or die 'open failed';
    print { $fh } $lines[0];
    syswrite($fh, $lines[1]);
    printf { $fh } '%s', $lines[2];
    close $fh or die 'close failed';

    $check_result->($filename, 'std functions');

    unlink $filename or die "cannot unlink $filename";

    $fh = Dpkg::Compression::FileHandle->new();
    $fh->open($filename, 'w');
    $fh->print($lines[0]);
    $fh->write($lines[1], length($lines[1]));
    $fh->printf('%s', $lines[2]);
    $fh->close() or die 'close failed';

    $check_result->($filename, 'IO::Handle methods');
}

sub check_uncompressed {
    my ($filename, $method) = @_;
    open(my $read_fh, '<', $filename) or die "cannot read $filename";
    my @read = <$read_fh>;
    close $read_fh or die 'cannot close';
    is_deeply(\@lines, \@read, "$filename correctly written ($method)");
}

sub check_compressed {
    my ($filename, $method) = @_;
    open my $read_fh, '-|', 'gunzip', '-c', "$tmpdir/myfile.gz"
        or die 'cannot fork zcat';
    my @read = <$read_fh>;
    close $read_fh or die 'cannot close';
    is_deeply(\@lines, \@read, "$filename correctly written ($method)");
}

sub test_read {
    my $filename = shift;

    $fh = Dpkg::Compression::FileHandle->new();
    open($fh, '<', $filename) or die 'open failed';
    my @read = <$fh>;
    close $fh or die 'close failed';

    is_deeply(\@lines, \@read, "$filename correctly read (std functions)");

    @read = ();
    $fh = Dpkg::Compression::FileHandle->new();
    $fh->open($filename, 'r') or die 'open failed';
    @read = $fh->getlines();
    $fh->close() or die 'close failed';

    is_deeply(\@lines, \@read, "$filename correctly read (IO::Handle methods)");
}

# Check compression properties.

my @compressors = compression_get_list();
is_deeply([ sort @compressors ] , [ qw(bzip2 gzip lzma xz) ],
    'supported compressors');

is(compression_get_default(), 'xz', 'default compressor is xz');
eval {
    compression_set_default('invented-compressor');
};
ok($@, 'cannot set compressor to an unsupported name');
is(compression_get_default(), 'xz', 'default compressor is still xz');
compression_set_default('gzip');
is(compression_get_default(), 'gzip', 'default compressor changed to gzip');
compression_set_default('xz');
is(compression_get_default(), 'xz', 'default compressor reset to xz');

ok(compression_is_supported('gzip'), 'gzip is supported');
ok(compression_is_supported('xz'), 'xz is supported');
ok(compression_is_supported('bzip2'), 'bzip2 is supported');
ok(compression_is_supported('lzma'), 'lzma is supported');

is(compression_guess_from_filename('filename'), undef,
    'compressor <none> guessed from "filename"');
is(compression_guess_from_filename('filename.gz'), 'gzip',
    'compressor <none> guessed from "filename"');
is(compression_guess_from_filename('filename.xz'), 'xz',
    'compressor <none> guessed from "filename"');
is(compression_guess_from_filename('filename.bz2'), 'bzip2',
    'compressor <none> guessed from "filename"');
is(compression_guess_from_filename('filename.lzma'), 'lzma',
    'compressor <none> guessed from "filename"');

is(compression_get_file_extension('gzip'), 'gz', 'gzip file ext');
is(compression_get_file_extension('xz'), 'xz', 'xz file ext');
is(compression_get_file_extension('bzip2'), 'bz2', 'bzip2 file ext');
is(compression_get_file_extension('lzma'), 'lzma', 'lzma file ext');

is(compression_get_level('gzip'), 9, 'gzip level is 9');
compression_set_level('gzip', 1);
is(compression_get_level('gzip'), 1, 'gzip level is now 1');
compression_set_level('gzip');
is(compression_get_level('gzip'), 9, 'gzip level is back to 9');
is(compression_get_level('xz'), 6, 'xz level is 6');
is(compression_get_level('bzip2'), 9, 'bzip2 level is 9');
is(compression_get_level('lzma'), 6, 'lzma level is 6');

my $ext_regex = compression_get_file_extension_regex();

ok('filename.gz' =~ m/\.$ext_regex$/, '.gz matches regex');
ok('filename.xz' =~ m/\.$ext_regex$/, '.xz matches regex');
ok('filename.bz2' =~ m/\.$ext_regex$/, '.bz2 matches regex');
ok('filename.lzma' =~ m/\.$ext_regex$/, '.lzma matches regex');

# Test changing the default compression levels
my $old_level = compression_get_default_level();
compression_set_default_level(1);
is(compression_get_default_level(), 1, 'change default compression level');
compression_set_default_level(5);
is(compression_get_default_level(), 5, 'change default compression level');
compression_set_default_level(undef);
is(compression_get_default_level(), $old_level, 'reset default compression level');

ok(! compression_is_valid_level(0), 'compression 0 is invalid');
ok(compression_is_valid_level(1), 'compression 1 is valid');
ok(compression_is_valid_level(5), 'compression 5 is valid');
ok(compression_is_valid_level(9), 'compression 9 is valid');
ok(compression_is_valid_level('fast'), 'compression fast is valid');
ok(compression_is_valid_level('best'), 'compression best is valid');

# Test write on uncompressed file
test_write("$tmpdir/myfile", \&check_uncompressed);

SKIP: {
    skip 'gunzip not available', 1 if not $have_gunzip;

    # Test write on compressed file
    test_write("$tmpdir/myfile.gz", \&check_compressed);
}

# Test read on uncompressed file
test_read("$tmpdir/myfile");

# Test read on compressed file
test_read("$tmpdir/myfile.gz");
