#!/usr/bin/perl
#
# dpkg-ar
#
# Copyright © 2024 Guillem Jover <guillem@debian.org>
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
use Dpkg::Archive::Ar;

textdomain('dpkg-dev');

my $action;

sub version
{
    printf(g_("Debian %s version %s.\n"), $Dpkg::PROGNAME, $Dpkg::PROGVERSION);
}

sub usage
{
    printf(g_("Usage: %s [<option>...]\n"), $Dpkg::PROGNAME);

    print(g_('
Commands:
      --create <archive> <file>...      create an ar archive.
      --list <archive>                  list the contents of an ar archive.
      --extract <archive> [<file>...]   extract the contents of an ar archive.
  -?, --help                            show this help message.
      --version                         show the version.
'));
}

sub create
{
    my ($archive, @files) = @_;

    my $ar = Dpkg::Archive::Ar->new(
        filename => $archive,
        create => 1,
    );

    foreach my $file (@files) {
        $ar->add_file($file);
    }
}

sub list
{
    my $archive = shift;

    my $ar = Dpkg::Archive::Ar->new(filename => $archive);

    foreach my $member (@{$ar->get_members()}) {
        print "$member->{name}\n";
    }
}

sub extract
{
    my ($archive, @files) = @_;
    my %file = map { $_ => 1 } @files;

    my $ar = Dpkg::Archive::Ar->new(filename => $archive);

    foreach my $member (@{$ar->get_members()}) {
        next if @files && ! exists $file{$member->{name}};

        $ar->extract_member($member);
    }
}

my @files;

while (@ARGV) {
    my $arg = shift @ARGV;
    if ($arg eq '-?' or $arg eq '--help') {
        usage();
        exit(0);
    } elsif ($arg eq '-v' or $arg eq '--version') {
        version();
        exit(0);
    } elsif ($arg eq '--create') {
        $action = 'create';
    } elsif ($arg eq '--list') {
        $action = 'list';
    } elsif ($arg eq '--extract') {
        $action = 'extract';
    } elsif ($arg eq '--') {
        push @files, @ARGV;
        last;
    } elsif ($arg =~ m/^-/) {
        usageerr(g_("unknown option '%s'"), $arg);
    } else {
        push @files, $arg;
    }
}

@files or usageerr(g_('need at least an archive filename'));

if ($action eq 'create') {
    if (@files == 1) {
        usageerr(g_('need at least a file to add into the archive'));
    }
    create(@files);
} elsif ($action eq 'list') {
    list(@files);
} elsif ($action eq 'extract') {
    extract(@files);
}
