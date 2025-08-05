#!/usr/bin/perl
#
# dpkg-buildapi
#
# Copyright © 2022 Guillem Jover <guillem@debian.org>
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

use warnings;
use strict;

use Dpkg ();
use Dpkg::Gettext;
use Dpkg::ErrorHandling;
use Dpkg::BuildAPI qw(get_build_api);
use Dpkg::Control::Info;

textdomain('dpkg-dev');

sub version
{
    printf(g_("Debian %s version %s.\n"), $Dpkg::PROGNAME, $Dpkg::PROGVERSION);
}

sub usage
{
    printf g_(
'Usage: %s [<option>...] [<command>]')
    . "\n\n" . g_(
'Commands:
  -?, --help               show this help message.
      --version            show the version.')
    . "\n\n" . g_(
'Options:
  -c<control-file>         get control info from this file.
'), $Dpkg::PROGNAME;
}

my $controlfile = 'debian/control';

while (@ARGV) {
    $_ = shift(@ARGV);
    if (m/^-\?|--help$/) {
        usage();
        exit 0;
    } elsif (m/^--version$/) {
        version();
        exit 0;
    } elsif (m/-c(.*)$/) {
        $controlfile = $1;
    } elsif (m/^--$/) {
        last;
    } elsif (m/^-/) {
        usageerr(g_("unknown option '%s'"), $_);
    } else {
        usageerr(g_('no arguments accepted'));
    }
}

my $ctrl = Dpkg::Control::Info->new($controlfile);

print get_build_api($ctrl) . "\n";

0;
