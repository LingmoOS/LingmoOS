# Copyright © 2015 Guillem Jover <guillem@debian.org>
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

Test::Dpkg - helpers for test scripts for the dpkg suite

=head1 DESCRIPTION

This module provides helper functions to ease implementing test scripts
for the dpkg suite of tools.

B<Note>: This is a private module, its API can change at any time.

=cut

package Test::Dpkg 0.00;

use strict;
use warnings;

our @EXPORT_OK = qw(
    all_po_files
    all_perl_files
    all_perl_modules
    test_get_po_dirs
    test_get_perl_dirs
    test_get_data_path
    test_get_temp_path
    test_needs_author
    test_needs_module
    test_needs_command
    test_needs_openpgp_backend
    test_needs_srcdir_switch
    test_neutralize_checksums
);
our %EXPORT_TAGS = (
    needs => [ qw(
        test_needs_author
        test_needs_module
        test_needs_command
        test_needs_openpgp_backend
        test_needs_srcdir_switch
    ) ],
    paths => [ qw(
        all_po_files
        all_perl_files
        all_perl_modules
        test_get_po_dirs
        test_get_perl_dirs
        test_get_data_path
        test_get_temp_path
    ) ],
);

use Exporter qw(import);
use Cwd;
use File::Find;
use File::Basename;
use File::Path qw(make_path rmtree);
use IPC::Cmd qw(can_run);
use Test::More;

my $test_mode;

BEGIN {
    $test_mode = $ENV{DPKG_TEST_MODE} // 'dpkg';
}

sub _test_get_caller_dir
{
    my (undef, $path, undef) = caller 1;

    $path =~ s{\.t$}{};
    $path =~ s{^\./}{};

    return $path;
}

sub test_get_data_path
{
    my $path = shift;

    if (defined $path) {
        my $srcdir;
        $srcdir = $ENV{srcdir} if $test_mode ne 'cpan';
        $srcdir ||= '.';
        return "$srcdir/$path";
    } else {
        return _test_get_caller_dir();
    }
}

sub test_get_temp_path
{
    my $path = shift // _test_get_caller_dir();
    $path = 't.tmp/' . fileparse($path);

    rmtree($path);
    make_path($path);
    return $path;
}

sub test_get_po_dirs
{
    if ($test_mode eq 'cpan') {
        return qw();
    } else {
        return qw(po scripts/po dselect/po man/po);
    }
}

sub test_get_perl_dirs
{
    if ($test_mode eq 'cpan') {
        return qw(t lib);
    } else {
        return qw(t lib utils/t scripts dselect);
    }
}

sub _test_get_files
{
    my ($filter, $dirs) = @_;
    my @files;
    my $scan_files = sub {
        push @files, $File::Find::name if m/$filter/;
    };

    find($scan_files, @{$dirs});

    return @files;
}

sub all_po_files
{
    return _test_get_files(qr/\.(?:po|pot)$/, [ test_get_po_dirs() ]);
}

sub all_perl_files
{
    return _test_get_files(qr/\.(?:PL|pl|pm|t)$/, [ test_get_perl_dirs() ]);
}

sub all_perl_modules
{
    return _test_get_files(qr/\.pm$/, [ test_get_perl_dirs() ]);
}

sub test_needs_author
{
    if (not $ENV{AUTHOR_TESTING}) {
        plan skip_all => 'author test';
    }
}

sub test_needs_module
{
    my ($module, @imports) = @_;
    my ($package) = caller;

    require version;
    my $version = '';
    if (@imports >= 1 and version::is_lax($imports[0])) {
        $version = shift @imports;
    }

    eval qq{
        package $package;
        use $module $version \@imports;
        1;
    } or do {
        plan skip_all => "requires module $module $version";
    }
}

sub test_needs_command
{
    my $command = shift;

    if (not can_run($command)) {
        plan skip_all => "requires command $command";
    }
}

my @openpgp_backends = (
    {
        backend => 'gpg',
        cmd => 'gpg-sq',
        cmdv => 'gpgv-sq',
    },
    {
        backend => 'gpg',
        cmd => 'gpg',
        cmdv => 'gpgv',
    },
    {
        backend => 'sq',
        cmd => 'sq',
        cmdv => 'sqv',
    },
    {
        backend => 'sop',
        cmd => 'sop',
        cmdv => 'sopv',
    },
    {
        backend => 'sop',
        cmd => 'sqop',
        cmdv => 'sqopv',
    },
    {
        backend => 'sop',
        cmd => 'rsop',
        cmdv => 'rsopv',
    },
    {
        backend => 'sop',
        cmd => 'gosop',
    },
    {
        backend => 'sop',
        cmd => 'pgpainless-cli',
    },
);

sub test_needs_openpgp_backend
{
    my @have_backends;
    foreach my $backend (@openpgp_backends) {
        my $name = $backend->{backend};
        my $cmd = $backend->{cmd} // q();
        my $cmdv = $backend->{cmdv} // q();

        my $have_cmd = $cmd eq 'none' ? 0 : can_run($cmd);
        my $have_cmdv = $cmdv eq 'none' ? 0 : can_run($cmdv);

        next unless ($have_cmd || $have_cmdv);

        my $have_backend = {
            backend => $name,
        };
        $have_backend->{cmd} = $cmd if $have_cmd;
        $have_backend->{cmdv} = $cmdv if $have_cmdv;

        push @have_backends, $have_backend;

        if ($have_cmd && $have_cmdv) {
            push @have_backends, {
                backend => $name,
                cmd => $cmd,
                cmdv => 'none',
            };
            push @have_backends, {
                backend => $name,
                cmd => 'none',
                cmdv => $cmdv,
            };
        }
    }
    if (@have_backends == 0) {
        my @cmds = grep {
            $_ ne 'none'
        } map {
            ( $_->{cmd}, $_->{cmdv} )
        } @openpgp_backends;
        plan skip_all => "requires >= 1 openpgp command: @cmds";
    }

    return @have_backends;
}

sub test_needs_srcdir_switch
{
    if (defined $ENV{srcdir}) {
        chdir $ENV{srcdir} or BAIL_OUT("cannot chdir to source directory: $!");
    }
}

sub test_neutralize_checksums
{
    my $filename = shift;
    my $filenamenew = "$filename.new";

    my $cwd = getcwd();
    open my $fhnew, '>', $filenamenew or die "cannot open new $filenamenew in $cwd: $!";
    open my $fh, '<', $filename or die "cannot open $filename in $cwd: $!";
    while (<$fh>) {
        s/^ ([0-9a-f]{32,}) [1-9][0-9]* /q{ } . $1 =~ tr{0-9a-f}{0}r . q{ 0 }/e;
        print { $fhnew } $_;
    }
    close $fh or die "cannot close $filename";
    close $fhnew or die "cannot close $filenamenew";

    rename $filenamenew, $filename or die "cannot rename $filenamenew to $filename";
}

=head1 CHANGES

=head2 Version 0.xx

This is a private module.

=cut

1;
