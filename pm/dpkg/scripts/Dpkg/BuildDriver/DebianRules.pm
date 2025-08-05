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

Dpkg::BuildDriver::DebianRules - build a Debian package using debian/rules

=head1 DESCRIPTION

This class is used by dpkg-buildpackage to drive the build of a Debian
package, using F<debian/rules>.

B<Note>: This is a private module, its API can change at any time.

=cut

package Dpkg::BuildDriver::DebianRules 0.01;

use strict;
use warnings;

use Dpkg ();
use Dpkg::Gettext;
use Dpkg::ErrorHandling;
use Dpkg::Path qw(find_command);
use Dpkg::BuildTypes;

=head1 METHODS

=over 4

=item $bd = Dpkg::BuildDriver::DebianRules->new(%opts)

Create a new Dpkg::BuildDriver::DebianRules object.

Supports or requires the same Dpkg::BuildDriver->new() options.

=cut

sub new {
    my ($this, %opts) = @_;
    my $class = ref($this) || $this;

    my $self = {
        ctrl => $opts{ctrl},
        root_cmd => $opts{root_cmd} // [],
        as_root => $opts{as_root},
        debian_rules => $opts{debian_rules},
        rrr_override => $opts{rrr_override},
    };
    bless $self, $class;

    my $rrr = $self->_parse_rules_requires_root();

    $self->{rules_requires_root} = $rrr;

    return $self;
}

sub _setup_rootcommand {
    my $self = shift;

    if ($< == 0) {
        warning(g_('using a gain-root-command while being root'))
            if @{$self->{root_cmd}};
    } else {
        push @{$self->{root_cmd}}, 'fakeroot'
            unless @{$self->{root_cmd}};
    }

    if (@{$self->{root_cmd}} && ! find_command($self->{root_cmd}[0])) {
        if ($self->{root_cmd}[0] eq 'fakeroot' && $< != 0) {
            error(g_("fakeroot not found, either install the fakeroot\n" .
                     'package, specify a command with the -r option, ' .
                     'or run this as root'));
        } else {
            error(g_("gain-root-command '%s' not found"), $self->{root_cmd}[0]);
        }
    }
}

my %target_build = map { $_ => 1 } qw(
    build
    build-arch
    build-indep
);
my %target_legacy_root = map { $_ => 1 } qw(
    clean
    binary
    binary-arch
    binary-indep
);
my %target_official =  map { $_ => 1 } qw(
    clean
    build
    build-arch
    build-indep
    binary
    binary-arch
    binary-indep
);

# Check whether we are doing some kind of rootless build, and sanity check
# the fields values.
sub _parse_rules_requires_root {
    my $self = shift;

    my %rrr;
    my $rrr;
    my $rrr_default;
    my $keywords_base = 0;
    my $keywords_impl = 0;

    $rrr_default = 'no';

    my $ctrl_src = $self->{ctrl}->get_source();
    $rrr = $self->{rrr_override} // $ctrl_src->{'Rules-Requires-Root'} // $rrr_default;

    foreach my $keyword (split ' ', $rrr) {
        if ($keyword =~ m{/}) {
            if ($keyword =~ m{^dpkg/target/(.*)$}p) {
                error(g_('disallowed target in %s field keyword %s'),
                      'Rules-Requires-Root', $keyword)
                    if $target_official{$1};
            } elsif ($keyword =~ m{^dpkg/(.*)$} and $1 ne 'target-subcommand') {
                error(g_('%s field keyword "%s" is unknown in dpkg namespace'),
                      'Rules-Requires-Root', $keyword);
            }
            $keywords_impl++;
        } else {
            if ($keyword ne lc $keyword and
                (lc $keyword eq 'no' or lc $keyword eq 'binary-targets')) {
                error(g_('%s field keyword "%s" is uppercase; use "%s" instead'),
                      'Rules-Requires-Root', $keyword, lc $keyword);
            } elsif (lc $keyword eq 'yes') {
                error(g_('%s field keyword "%s" is invalid; use "%s" instead'),
                      'Rules-Requires-Root', $keyword, 'binary-targets');
            } elsif ($keyword ne 'no' and $keyword ne 'binary-targets') {
                warning(g_('%s field keyword "%s" is unknown'),
                        'Rules-Requires-Root', $keyword);
            }
            $keywords_base++;
        }

        if ($rrr{$keyword}++) {
            error(g_('field %s contains duplicate keyword %s'),
                  'Rules-Requires-Root', $keyword);
        }
    }

    if ($self->{as_root} || ! exists $rrr{no}) {
        $self->_setup_rootcommand();
    }

    # Notify the children we do support R³.
    $ENV{DEB_RULES_REQUIRES_ROOT} = join ' ', sort keys %rrr;

    if ($keywords_base > 1 or $keywords_base and $keywords_impl) {
        error(g_('%s field contains both global and implementation specific keywords'),
              'Rules-Requires-Root');
    } elsif ($keywords_impl) {
        # Set only on <implementations-keywords>.
        $ENV{DEB_GAIN_ROOT_CMD} = join ' ', @{$self->{root_cmd}};
    } else {
        # We should not provide the variable otherwise.
        delete $ENV{DEB_GAIN_ROOT_CMD};
    }

    return \%rrr;
}

sub _rules_requires_root {
    my ($self, $target) = @_;

    return 1 if $self->{as_root};
    return 1 if $self->{rules_requires_root}{"dpkg/target/$target"};
    return 1 if $self->{rules_requires_root}{'binary-targets'} and $target_legacy_root{$target};
    return 0;
}

sub _run_cmd {
    my @cmd = @_;
    printcmd(@cmd);
    system @cmd and subprocerr("@cmd");
}

sub _run_rules_cond_root {
    my ($self, $target) = @_;

    my @cmd;
    push @cmd, @{$self->{root_cmd}} if $self->_rules_requires_root($target);
    push @cmd, @{$self->{debian_rules}}, $target;

    _run_cmd(@cmd);
}

=item $bd->pre_check()

Perform build driver specific checks, before anything else.

This checks whether the F<debian/rules> file is executable,
and if not then make it so.

=cut

sub pre_check {
    my $self = shift;

    if (@{$self->{debian_rules}} == 1 && ! -x $self->{debian_rules}[0]) {
        warning(g_('%s is not executable; fixing that'),
                $self->{debian_rules}[0]);
        # No checks of failures, non fatal.
        chmod 0755, $self->{debian_rules}[0];
    }
}

=item $bool = $bd->need_build_task($build_task, $binary_task)

Returns whether we need to use the build task.

B<Note>: This method is needed as long as we support building as root-like.
Once that is not needed this method will be deprecated.

=cut

sub need_build_task {
    my ($self, $build_task, $binary_task) = @_;

    # If we are building rootless, there is no need to call the build target
    # independently as non-root.
    return 0 if not $self->_rules_requires_root($binary_task);
    return 1;
}

=item $bd->run_build_task($build_task, $binary_task)

Executes the build task for the build.

B<Note>: This method is needed as long as we support building as root-like.
Once that is not needed this method will be deprecated.

=cut

sub run_build_task {
    my ($self, $build_task, $binary_task) = @_;

    # If we are building rootless, there is no need to call the build
    # target independently as non-root.
    if ($self->_rules_requires_root($binary_task)) {
        _run_cmd(@{$self->{debian_rules}}, $build_task)
    }

    return;
}

=item $bd->run_task($task)

Executes the given task for the build.

=cut

sub run_task {
    my ($self, $task) = @_;

    $self->_run_rules_cond_root($task);

    return;
}

=back

=head1 CHANGES

=head2 Version 0.xx

This is a private module.

=cut

1;
