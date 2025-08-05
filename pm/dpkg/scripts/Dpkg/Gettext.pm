# Based on Debconf::Gettext.
#
# Copyright © 2000 Joey Hess <joeyh@debian.org>
# Copyright © 2006 Nicolas François <nicolas.francois@centraliens.net>
# Copyright © 2007-2022 Guillem Jover <guillem@debian.org>
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

Dpkg::Gettext - convenience wrapper around Locale::gettext

=head1 DESCRIPTION

The Dpkg::Gettext module is a convenience wrapper over the L<Locale::gettext>
module, to guarantee we always have working gettext functions, and to add
some commonly used aliases.

=cut

package Dpkg::Gettext 2.01;

use strict;
use warnings;
use feature qw(state);

our @EXPORT = qw(
    textdomain
    gettext
    ngettext
    g_
    P_
    N_
);

use Exporter qw(import);

=head1 ENVIRONMENT

=over 4

=item DPKG_NLS

When set to 0, this environment variable will disable the National Language
Support in all Dpkg modules.

=back

=head1 VARIABLES

=over 4

=item $Dpkg::Gettext::DEFAULT_TEXT_DOMAIN

Specifies the default text domain name to be used with the short function
aliases. This is intended to be used by the Dpkg modules, so that they
can produce localized messages even when the calling program has set the
current domain with textdomain(). If you would like to use the aliases
for your own modules, you might want to set this variable to undef, or
to another domain, but then the Dpkg modules will not produce localized
messages.

=back

=cut

our $DEFAULT_TEXT_DOMAIN = 'dpkg-dev';

=head1 FUNCTIONS

=over 4

=item $domain = textdomain($new_domain)

Compatibility textdomain() fallback when L<Locale::gettext> is not available.

If $new_domain is not undef, it will set the current domain to $new_domain.
Returns the current domain, after possibly changing it.

=item $trans = gettext($msgid)

Compatibility gettext() fallback when L<Locale::gettext> is not available.

Returns $msgid.

=item $trans = ngettext($msgid, $msgid_plural, $n)

Compatibility ngettext() fallback when L<Locale::gettext> is not available.

Returns $msgid if $n is 1 or $msgid_plural otherwise.

=item $trans = g_($msgid)

Calls dgettext() on the $msgid and returns its translation for the current
locale. If dgettext() is not available, simply returns $msgid.

=item $trans = C_($msgctxt, $msgid)

Calls dgettext() on the $msgid and returns its translation for the specific
$msgctxt supplied. If dgettext() is not available, simply returns $msgid.

=item $trans = P_($msgid, $msgid_plural, $n)

Calls dngettext(), returning the correct translation for the plural form
dependent on $n. If dngettext() is not available, returns $msgid if $n is 1
or $msgid_plural otherwise.

=cut

use constant GETTEXT_CONTEXT_GLUE => "\004";

BEGIN {
    my $use_gettext = $ENV{DPKG_NLS} // 1;
    if ($use_gettext) {
        eval q{
            use Locale::gettext;
        };
        $use_gettext = not $@;
    }
    if (not $use_gettext) {
        *g_ = sub {
            return shift;
        };
        *textdomain = sub {
            my $new_domain = shift;
            state $domain = $DEFAULT_TEXT_DOMAIN;

            $domain = $new_domain if defined $new_domain;

            return $domain;
        };
        *gettext = sub {
            my $msgid = shift;
            return $msgid;
        };
        *ngettext = sub {
            my ($msgid, $msgid_plural, $n) = @_;
            if ($n == 1) {
                return $msgid;
            } else {
                return $msgid_plural;
            }
        };
        *C_ = sub {
            my ($msgctxt, $msgid) = @_;
            return $msgid;
        };
        *P_ = sub {
            return ngettext(@_);
        };
    } else {
        *g_ = sub {
            return dgettext($DEFAULT_TEXT_DOMAIN, shift);
        };
        *C_ = sub {
            my ($msgctxt, $msgid) = @_;
            return dgettext($DEFAULT_TEXT_DOMAIN,
                            $msgctxt . GETTEXT_CONTEXT_GLUE . $msgid);
        };
        *P_ = sub {
            return dngettext($DEFAULT_TEXT_DOMAIN, @_);
        };
    }
}

=item $msgid = N_($msgid)

A pseudo function that servers as a marker for automated extraction of
messages, but does not call gettext(). The run-time translation is done
at a different place in the code.

=back

=cut

sub N_
{
    my $msgid = shift;
    return $msgid;
}

=head1 CHANGES

=head2 Version 2.01 (dpkg 1.21.10)

New function: gettext().

=head2 Version 2.00 (dpkg 1.20.0)

Remove function: _g().

=head2 Version 1.03 (dpkg 1.19.0)

New envvar: Add support for new B<DPKG_NLS> environment variable.

=head2 Version 1.02 (dpkg 1.18.3)

New function: N_().

=head2 Version 1.01 (dpkg 1.18.0)

Now the short aliases (g_ and P_) will call domain aware functions with
$DEFAULT_TEXT_DOMAIN.

New functions: g_(), C_().

Deprecated function: _g().

=head2 Version 1.00 (dpkg 1.15.6)

Mark the module as public.

=cut

1;
