# Copyright © 2023 Guillem Jover <guillem@debian.org>
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

Dpkg::BuildTree - handle build tree actions

=head1 DESCRIPTION

The Dpkg::BuildTree module provides functions to handle build tree actions.

B<Note>: This is a private module, its API can change at any time.

=cut

package Dpkg::BuildTree 0.01;

use strict;
use warnings;

use Cwd;

use Dpkg::Source::Functions qw(erasedir);

=head1 METHODS

=over 4

=item $bt = Dpkg::BuildTree->new(%opts)

Create a new Dpkg::BuildTree object.

Options:

=over 8

=item B<dir>

The build tree directory.
If not specified, it assumes the current working directory.

=back

=cut

sub new {
    my ($this, %opts) = @_;
    my $class = ref($this) || $this;

    my $self = {
        buildtree => $opts{dir} || getcwd(),
    };
    bless $self, $class;

    return $self;
}

=item $bt->clean()

Clean the build tree, by removing any dpkg generated artifacts.

=cut

sub clean {
    my $self = shift;

    my $buildtree = $self->{buildtree};

    # If this does not look like a build tree, do nothing.
    return unless -f "$buildtree/debian/control";

    my @files = qw(
        debian/files
        debian/files.new
        debian/substvars
        debian/substvars.new
    );
    my @dirs = qw(
        debian/tmp
    );

    foreach my $file (@files) {
        unlink "$buildtree/$file";
    }
    foreach my $dir (@dirs) {
        erasedir("$buildtree/$dir");
    }

    return;
}

=item $bt->needs_root()

Check whether the build tree needs root to build.

=cut

sub needs_root {
    my $self = shift;

    require Dpkg::Control::Info;
    require Dpkg::BuildDriver;

    my $ctrl = Dpkg::Control::Info->new();
    my $bd = Dpkg::BuildDriver->new(ctrl => $ctrl);

    return $bd->need_build_task('build', 'binary') ||
           defined $ENV{DEB_GAIN_ROOT_CMD};
}

=back

=head1 CHANGES

=head2 Version 0.xx

This is a private module.

=cut

1;
