# Copyright © 2007-2022 Guillem Jover <guillem@debian.org>
# Copyright © 2010 Raphaël Hertzog <hertzog@debian.org>
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

Dpkg::Compression - simple database of available compression methods

=head1 DESCRIPTION

This modules provides a few public functions and a public regex to
interact with the set of supported compression methods.

=cut

package Dpkg::Compression 2.01;

use strict;
use warnings;

our @EXPORT = qw(
    compression_is_supported
    compression_get_list
    compression_get_property
    compression_guess_from_filename
    compression_get_file_extension_regex
    compression_get_file_extension
    compression_get_default
    compression_set_default
    compression_get_default_level
    compression_set_default_level
    compression_get_level
    compression_set_level
    compression_is_valid_level
    compression_get_threads
    compression_set_threads
    compression_get_cmdline_compress
    compression_get_cmdline_decompress
);

use Exporter qw(import);
use Config;
use List::Util qw(any);

use Dpkg::ErrorHandling;
use Dpkg::Gettext;

my %COMP = (
    gzip => {
	file_ext => 'gz',
	comp_prog => [ 'gzip', '-n' ],
	decomp_prog => [ 'gunzip' ],
	default_level => 9,
    },
    bzip2 => {
	file_ext => 'bz2',
	comp_prog => [ 'bzip2' ],
	decomp_prog => [ 'bunzip2' ],
	default_level => 9,
    },
    lzma => {
	file_ext => 'lzma',
	comp_prog => [ 'xz', '--format=lzma' ],
	decomp_prog => [ 'unxz', '--format=lzma' ],
	default_level => 6,
    },
    xz => {
	file_ext => 'xz',
	comp_prog => [ 'xz' ],
	decomp_prog => [ 'unxz' ],
	default_level => 6,
    },
);

# The gzip --rsyncable option is not universally supported, so we need to
# conditionally use it. Ideally we would invoke 'gzip --help' and check
# whether the option is supported, but that would imply forking and executing
# that process for any module that ends up loading this one, which is not
# acceptable performance-wise. Instead we will approximate it by osname, which
# is not ideal, but better than nothing.
#
# Requires GNU gzip >= 1.7 for the --rsyncable option. On AIX GNU gzip is
# too old. On the BSDs they use their own implementation based on zlib,
# which does not currently support the --rsyncable option.
if (any { $Config{osname} eq $_ } qw(linux gnu solaris)) {
    push @{$COMP{gzip}{comp_prog}}, '--rsyncable';
}

my $default_compression = 'xz';
my $default_compression_level = undef;
my $default_compression_threads = 0;

my $regex = join '|', map { $_->{file_ext} } values %COMP;
my $compression_re_file_ext = qr/(?:$regex)/;

=head1 FUNCTIONS

=over 4

=item @list = compression_get_list()

Returns a list of supported compression methods (sorted alphabetically).

=cut

sub compression_get_list {
    my @list = sort keys %COMP;
    return @list;
}

=item compression_is_supported($comp)

Returns a boolean indicating whether the give compression method is
known and supported.

=cut

sub compression_is_supported {
    my $comp = shift;

    return exists $COMP{$comp};
}

=item compression_get_property($comp, $property)

Returns the requested property of the compression method. Returns undef if
either the property or the compression method doesn't exist. Valid
properties currently include "file_ext" for the file extension,
"default_level" for the default compression level,
"comp_prog" for the name of the compression program and "decomp_prog" for
the name of the decompression program.

This function is deprecated, please switch to one of the new specialized
getters instead.

=cut

sub compression_get_property {
    my ($comp, $property) = @_;

    #warnings::warnif('deprecated',
    #    'Dpkg::Compression::compression_get_property() is deprecated, ' .
    #    'use one of the specialized getters instead');
    return unless compression_is_supported($comp);
    return $COMP{$comp}{$property} if exists $COMP{$comp}{$property};
    return;
}

=item compression_guess_from_filename($filename)

Returns the compression method that is likely used on the indicated
filename based on its file extension.

=cut

sub compression_guess_from_filename {
    my $filename = shift;
    foreach my $comp (compression_get_list()) {
        my $ext = $COMP{$comp}{file_ext};
        if ($filename =~ /^(.*)\.\Q$ext\E$/) {
	    return $comp;
        }
    }
    return;
}

=item $regex = compression_get_file_extension_regex()

Returns a regex that matches a file extension of a file compressed with
one of the supported compression methods.

=cut

sub compression_get_file_extension_regex {
    return $compression_re_file_ext;
}

=item $ext = compression_get_file_extension($comp)

Return the file extension for the compressor $comp.

=cut

sub compression_get_file_extension {
    my $comp = shift;

    error(g_('%s is not a supported compression'), $comp)
        unless compression_is_supported($comp);

    return $COMP{$comp}{file_ext};
}

=item $comp = compression_get_default()

Return the default compression method. It is "xz" unless
compression_set_default() has been used to change it.

=cut

sub compression_get_default {
    return $default_compression;
}

=item compression_set_default($comp)

Change the default compression method. Errors out if the
given compression method is not supported.

=cut

sub compression_set_default {
    my $method = shift;
    error(g_('%s is not a supported compression'), $method)
        unless compression_is_supported($method);
    $default_compression = $method;
}

=item $level = compression_get_default_level()

Return the global default compression level used when compressing data if
it has been set, otherwise the default level for the default compressor.

It's "9" for "gzip" and "bzip2", "6" for "xz" and "lzma", unless
compression_set_default_level() has been used to change it.

=cut

sub compression_get_default_level {
    if (defined $default_compression_level) {
        return $default_compression_level;
    } else {
        return $COMP{$default_compression}{default_level};
    }
}

=item compression_set_default_level($level)

Change the global default compression level. Passing undef as the level will
reset it to the global default compressor specific default, otherwise errors
out if the level is not valid (see compression_is_valid_level()).

=cut

sub compression_set_default_level {
    my $level = shift;
    error(g_('%s is not a compression level'), $level)
        if defined($level) and not compression_is_valid_level($level);
    $default_compression_level = $level;
}

=item $level = compression_get_level($comp)

Return the compression level used when compressing data with a specific
compressor. The value returned is the specific compression level if it has
been set, otherwise the global default compression level if it has been set,
falling back to the specific default compression level.

=cut

sub compression_get_level {
    my $comp = shift;

    error(g_('%s is not a supported compression'), $comp)
        unless compression_is_supported($comp);

    return $COMP{$comp}{level} //
           $default_compression_level //
           $COMP{$comp}{default_level};
}

=item compression_set_level($comp, $level)

Change the compression level for a specific compressor. Passing undef as
the level will reset it to the specific default compressor level, otherwise
errors out if the level is not valid (see compression_is_valid_level()).

=cut

sub compression_set_level {
    my ($comp, $level) = @_;

    error(g_('%s is not a supported compression'), $comp)
        unless compression_is_supported($comp);
    error(g_('%s is not a compression level'), $level)
        if defined $level && ! compression_is_valid_level($level);

    $COMP{$comp}{level} = $level;
}

=item compression_is_valid_level($level)

Returns a boolean indicating whether $level is a valid compression level
(it must be either a number between 1 and 9 or "fast" or "best")

=cut

sub compression_is_valid_level {
    my $level = shift;
    return $level =~ /^([1-9]|fast|best)$/;
}

=item $threads = compression_get_threads()

Return the number of threads to use for compression and decompression.

=cut

sub compression_get_threads {
    return $default_compression_threads;
}

=item compression_set_threads($threads)

Change the threads to use for compression and decompression. Passing C<undef>
or B<0> requests to use automatic mode, based on the current CPU cores on
the system.

=cut

sub compression_set_threads {
    my $threads = shift;

    error(g_('compression threads %s is not a number'), $threads)
        if defined $threads && $threads !~ m/^\d+$/;
    $default_compression_threads = $threads;
}

=item @exec = compression_get_cmdline_compress($comp)

Returns a list ready to be passed to exec(), its first element is the
program name for compression and the following elements are parameters
for the program.

When executed the program will act as a filter between its standard input
and its standard output.

=cut

sub compression_get_cmdline_compress {
    my $comp = shift;

    error(g_('%s is not a supported compression'), $comp)
        unless compression_is_supported($comp);

    my @prog = @{$COMP{$comp}{comp_prog}};
    my $level = compression_get_level($comp);
    if ($level =~ m/^[1-9]$/) {
        push @prog, "-$level";
    } else {
        push @prog, "--$level";
    }
    my $threads = compression_get_threads();
    if ($comp eq 'xz') {
        # Do not generate warnings when adjusting memory usage, nor
        # exit with non-zero due to those not emitted warnings.
        push @prog, qw(--quiet --no-warn);

        # Do not let xz fallback to single-threaded mode, to avoid
        # non-reproducible output.
        push @prog, '--no-adjust';

        # The xz -T1 option selects a single-threaded mode which generates
        # different output than in multi-threaded mode. To avoid the
        # non-reproducible output we pass -T+1 (supported with xz >= 5.4.0)
        # to request multi-threaded mode with a single thread.
        push @prog, $threads == 1 ? '-T+1' : "-T$threads";
    }
    return @prog;
}

=item @exec = compression_get_cmdline_decompress($comp)

Returns a list ready to be passed to exec(), its first element is the
program name for decompression and the following elements are parameters
for the program.

When executed the program will act as a filter between its standard input
and its standard output.

=cut

sub compression_get_cmdline_decompress {
    my $comp = shift;

    error(g_('%s is not a supported compression'), $comp)
        unless compression_is_supported($comp);

    my @prog = @{$COMP{$comp}{decomp_prog}};

    my $threads = compression_get_threads();
    if ($comp eq 'xz') {
        push @prog, "-T$threads";
    }

    return @prog;
}

=back

=head1 CHANGES

=head2 Version 2.01 (dpkg 1.21.14)

New functions: compression_get_file_extension(), compression_get_level(),
compression_set_level(), compression_get_cmdline_compress(),
compression_get_cmdline_decompress(), compression_get_threads() and
compression_set_threads().

Deprecated functions: compression_get_property().

=head2 Version 2.00 (dpkg 1.20.0)

Hide variables: $default_compression, $default_compression_level
and $compression_re_file_ext.

=head2 Version 1.02 (dpkg 1.17.2)

New function: compression_get_file_extension_regex()

Deprecated variables: $default_compression, $default_compression_level
and $compression_re_file_ext

=head2 Version 1.01 (dpkg 1.16.1)

Default compression level is not global any more, it is per compressor type.

=head2 Version 1.00 (dpkg 1.15.6)

Mark the module as public.

=cut

1;
