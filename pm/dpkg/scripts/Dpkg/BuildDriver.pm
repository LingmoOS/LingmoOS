# Copyright © 2020-2024 Guillem Jover <guillem@debian.org>
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

Dpkg::BuildDriver - drive the build of a Debian package

=head1 DESCRIPTION

This class is used by dpkg-buildpackage to drive the build of a Debian
package.

B<Note>: This is a private module, its API can change at any time.

=cut

package Dpkg::BuildDriver 0.01;

use strict;
use warnings;

use Dpkg ();
use Dpkg::Gettext;
use Dpkg::ErrorHandling;

=head1 METHODS

=over 4

=item $bd = Dpkg::BuildDriver->new(%opts)

Create a new Dpkg::BuildDriver object.
It will load a build driver module as requested in the B<Build-Drivers> field
in the $opts{ctrl} L<Dpkg::Control::Info> object or if not present,
it will fall back to load the default B<debian-rules> driver.

Options:

=over

=item B<ctrl> (required)

A L<Dpkg::Control::Info> object.

=item B<root_cmd>

A string with the gain-root-command to use when needing to execute a command
with root-like rights.
If needed and unset, it will default to L<fakeroot> if it is available or
the module will error out.

=item B<as_root>

A boolean to force F<debian/rules> target calls as root-like, even if they
would normally not require to be executed as root-like.
This option is applied to all targets globally.

B<Note>: This option is only relevant for drivers that use F<debian/rules>.

=item B<debian_rules>

An array containing the command to execute the F<debian/rules> file and any
additional arguments.
It defaults to B<debian/rules>.

B<Note>: This option is only relevant for drivers that use F<debian/rules>.

=item B<rrr_override>

A string that overrides the B<Rules-Requires-Root> field value.

B<Note>: This option is only relevant for drivers that use F<debian/rules>.

=back

=cut

sub _load_driver {
    my ($name, %opts) = @_;

    # Normalize the driver name.
    $name = join q{}, map { ucfirst lc } split /-/, $name;

    my $module = "Dpkg::BuildDriver::$name";
    eval qq{
        require $module;
    };
    error(g_('build driver %s is unknown: %s'), $name, $@) if $@;

    return $module->new(%opts);
}

sub new {
    my ($this, %opts) = @_;
    my $class = ref($this) || $this;

    my $ctrl_src = $opts{ctrl}->get_source();
    my $name = $ctrl_src->{'Build-Driver'} // 'debian-rules';
    my $self = {
        driver => _load_driver($name, %opts),
    };
    bless $self, $class;
    return $self;
}

=item $bd->pre_check()

Perform build driver specific checks, before anything else.

This will run after the B<init> hook, and before C<dpkg-source --before-build>.

B<Note>: This is an optional method that can be omitted from the driver
implementation.

=cut

sub pre_check {
    my $self = shift;

    return unless $self->{driver}->can('pre_check');
    return $self->{driver}->pre_check();
}

=item $bool = $bd->need_build_task($build_task, binary_task)

Returns whether we need to use the build task.

B<Note>: This method is needed as long as we support building as root-like.
Once that is not needed this method will be deprecated.

=cut

sub need_build_task {
    my ($self, $build_task, $binary_task) = @_;

    return $self->{driver}->need_build_task($build_task, $binary_task);
}

=item $bd->run_build_task($build_task, $binary_task)

Executes the build task for the build.

B<Note>: This is an optional method needed as long as we support building as
root-like.
Once that is not needed this method will be deprecated.

=cut

sub run_build_task {
    my ($self, $build_task, $binary_task) = @_;

    $self->{driver}->run_build_task($build_task, $binary_task);

    return;
}

=item $bd->run_task($task)

Executes the given task for the build.

=cut

sub run_task {
    my ($self, $task) = @_;

    $self->{driver}->run_task($task);

    return;
}

=back

=head1 CHANGES

=head2 Version 0.xx

This is a private module.

=cut

1;
