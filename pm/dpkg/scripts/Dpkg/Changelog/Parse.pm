# Copyright © 2005, 2007 Frank Lichtenheld <frank@lichtenheld.de>
# Copyright © 2009 Raphaël Hertzog <hertzog@debian.org>
# Copyright © 2010, 2012-2015 Guillem Jover <guillem@debian.org>
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

Dpkg::Changelog::Parse - generic changelog parser for dpkg-parsechangelog

=head1 DESCRIPTION

This module provides a set of functions which reproduce all the features
of dpkg-parsechangelog.

=cut

package Dpkg::Changelog::Parse 2.01;

use strict;
use warnings;

our @EXPORT = qw(
    changelog_parse
);

use Exporter qw(import);
use List::Util qw(none);

use Dpkg ();
use Dpkg::Gettext;
use Dpkg::ErrorHandling;
use Dpkg::Control::Changelog;

sub _changelog_detect_format {
    my $file = shift;
    my $format = 'debian';

    # Extract the format from the changelog file if possible
    if ($file ne '-') {
        local $_;

        open my $format_fh, '<', $file
            or syserr(g_('cannot open file %s'), $file);
        if (-s $format_fh > 4096) {
            seek $format_fh, -4096, 2
                or syserr(g_('cannot seek into file %s'), $file);
        }
        while (<$format_fh>) {
            $format = $1 if m/\schangelog-format:\s+([0-9a-z]+)\W/;
        }
        close $format_fh;
    }

    return $format;
}

=head1 FUNCTIONS

=over 4

=item $fields = changelog_parse(%opts)

This function will parse a changelog. In list context, it returns as many
L<Dpkg::Control> objects as the parser did create. In scalar context, it will
return only the first one. If the parser did not return any data, it will
return an empty list in list context or undef on scalar context. If the
parser failed, it will die. Any parse errors will be printed as warnings
on standard error, but this can be disabled by passing $opts{verbose} to 0.

The parsing itself is done by a parser module (searched in the standard
perl library directories. That module is named according to the format that
it is able to parse, with the name capitalized. By default it is either
L<Dpkg::Changelog::Debian> (from the "debian" format) or the format name looked
up in the 40 last lines of the changelog itself (extracted with this perl
regular expression "\schangelog-format:\s+([0-9a-z]+)\W"). But it can be
overridden with $opts{changelogformat}.

All the other keys in %opts are forwarded to the parser module constructor.

Options:

=over

=item B<file>

Set the changelog file to parse.
Defaults to F<debian/changelog>.

=item B<label>

Set the changelog name used in output messages.
Defaults to $opts{file}.

=item B<compression>

Set a boolean on whether to load the file without compression support.
If the file is the default compression is disabled,
otherwise the default is to enable compression.

=item B<changelogformat>

Set the changelog input format to use.

=item B<format>

Set the output format to use.
Defaults to "dpkg".

=item B<verbose>

Set whether to print any parse errors as warnings to standard error.
Defaults to true.

=back

=cut

sub changelog_parse {
    my (%opts) = @_;

    $opts{verbose} //= 1;
    $opts{file} //= 'debian/changelog';
    $opts{label} //= $opts{file};
    $opts{changelogformat} //= _changelog_detect_format($opts{file});
    $opts{format} //= 'dpkg';
    $opts{compression} //= $opts{file} ne 'debian/changelog';

    my @range_opts = qw(since until from to offset count reverse all);
    $opts{all} = 1 if exists $opts{all};
    if (none { defined $opts{$_} } @range_opts) {
        $opts{count} = 1;
    }
    my $range;
    foreach my $opt (@range_opts) {
        $range->{$opt} = $opts{$opt} if exists $opts{$opt};
    }

    # Find the right changelog parser.
    my $format = ucfirst lc $opts{changelogformat};
    my $changes;
    my $module = "Dpkg::Changelog::$format";
    eval qq{
        require $module;
    };
    error(g_('changelog format %s is unknown: %s'), $format, $@) if $@;
    $changes = $module->new();
    error(g_('changelog format %s is not a Dpkg::Changelog class'), $format)
        unless $changes->isa('Dpkg::Changelog');
    $changes->set_options(reportfile => $opts{label},
                          verbose => $opts{verbose},
                          range => $range);

    # Load and parse the changelog.
    $changes->load($opts{file}, compression => $opts{compression})
        or error(g_('fatal error occurred while parsing %s'), $opts{file});

    # Get the output into several Dpkg::Control objects.
    my @res;
    if ($opts{format} eq 'dpkg') {
        push @res, $changes->format_range('dpkg', $range);
    } elsif ($opts{format} eq 'rfc822') {
        push @res, $changes->format_range('rfc822', $range);
    } else {
        error(g_('unknown output format %s'), $opts{format});
    }

    if (wantarray) {
        return @res;
    } else {
        return $res[0] if @res;
        return;
    }
}

=back

=head1 CHANGES

=head2 Version 2.01 (dpkg 1.20.6)

New option: 'verbose' in changelog_parse().

=head2 Version 2.00 (dpkg 1.20.0)

Remove functions: changelog_parse_debian(), changelog_parse_plugin().

Remove warnings: For options 'forceplugin', 'libdir'.

=head2 Version 1.03 (dpkg 1.19.0)

New option: 'compression' in changelog_parse().

=head2 Version 1.02 (dpkg 1.18.8)

Deprecated functions: changelog_parse_debian(), changelog_parse_plugin().

Obsolete options: forceplugin, libdir.

=head2 Version 1.01 (dpkg 1.18.2)

New functions: changelog_parse_debian(), changelog_parse_plugin().

=head2 Version 1.00 (dpkg 1.15.6)

Mark the module as public.

=cut

1;
