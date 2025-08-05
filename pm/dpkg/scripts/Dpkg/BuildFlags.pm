# Copyright © 2010-2011 Raphaël Hertzog <hertzog@debian.org>
# Copyright © 2012-2022 Guillem Jover <guillem@debian.org>
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

Dpkg::BuildFlags - query build flags

=head1 DESCRIPTION

This class is used by dpkg-buildflags and can be used
to query the same information.

=cut

package Dpkg::BuildFlags 1.06;

use strict;
use warnings;

use Dpkg ();
use Dpkg::Gettext;
use Dpkg::BuildEnv;
use Dpkg::ErrorHandling;
use Dpkg::Vendor qw(run_vendor_hook);

=head1 METHODS

=over 4

=item $bf = Dpkg::BuildFlags->new(%opts)

Create a new Dpkg::BuildFlags object. It will be initialized based
on the value of several configuration files and environment variables.

Options:

=over

=item B<vendor_defaults>

If set to false, then no vendor defaults are initialized
(it defaults to true).

=back

=cut

sub new {
    my ($this, %opts) = @_;
    my $class = ref($this) || $this;

    my $self = {
    };
    bless $self, $class;

    $opts{vendor_defaults} //= 1;

    if ($opts{vendor_defaults}) {
        $self->load_vendor_defaults();
    } else {
        $self->_init_vendor_defaults();
    }
    return $self;
}

sub _init_vendor_defaults {
    my $self = shift;

    my @flags = qw(
        ASFLAGS
        ASFLAGS_FOR_BUILD
        CPPFLAGS
        CPPFLAGS_FOR_BUILD
        CFLAGS
        CFLAGS_FOR_BUILD
        CXXFLAGS
        CXXFLAGS_FOR_BUILD
        OBJCFLAGS
        OBJCFLAGS_FOR_BUILD
        OBJCXXFLAGS
        OBJCXXFLAGS_FOR_BUILD
        DFLAGS
        DFLAGS_FOR_BUILD
        FFLAGS
        FFLAGS_FOR_BUILD
        FCFLAGS
        FCFLAGS_FOR_BUILD
        LDFLAGS
        LDFLAGS_FOR_BUILD
    );

    $self->{features} = {};
    $self->{builtins} = {};
    $self->{optvals} = {};
    $self->{flags} = { map { $_ => '' } @flags };
    $self->{origin} = { map { $_ => 'vendor' } @flags };
    $self->{maintainer} = { map { $_ => 0 } @flags };
}

=item $bf->load_vendor_defaults()

Reset the flags stored to the default set provided by the vendor.

=cut

sub load_vendor_defaults {
    my $self = shift;

    $self->_init_vendor_defaults();

    # The vendor hook will add the feature areas build flags.
    run_vendor_hook('update-buildflags', $self);
}

=item $bf->load_system_config()

Update flags from the system configuration.

=cut

sub load_system_config {
    my $self = shift;

    $self->update_from_conffile("$Dpkg::CONFDIR/buildflags.conf", 'system');
}

=item $bf->load_user_config()

Update flags from the user configuration.

=cut

sub load_user_config {
    my $self = shift;

    my $confdir = $ENV{XDG_CONFIG_HOME};
    $confdir ||= $ENV{HOME} . '/.config' if length $ENV{HOME};
    if (length $confdir) {
        $self->update_from_conffile("$confdir/dpkg/buildflags.conf", 'user');
    }
}

=item $bf->load_environment_config()

Update flags based on user directives stored in the environment. See
L<dpkg-buildflags(1)> for details.

=cut

sub load_environment_config {
    my $self = shift;

    foreach my $flag (keys %{$self->{flags}}) {
	my $envvar = 'DEB_' . $flag . '_SET';
	if (Dpkg::BuildEnv::has($envvar)) {
	    $self->set($flag, Dpkg::BuildEnv::get($envvar), 'env');
	}
	$envvar = 'DEB_' . $flag . '_STRIP';
	if (Dpkg::BuildEnv::has($envvar)) {
	    $self->strip($flag, Dpkg::BuildEnv::get($envvar), 'env');
	}
	$envvar = 'DEB_' . $flag . '_APPEND';
	if (Dpkg::BuildEnv::has($envvar)) {
	    $self->append($flag, Dpkg::BuildEnv::get($envvar), 'env');
	}
	$envvar = 'DEB_' . $flag . '_PREPEND';
	if (Dpkg::BuildEnv::has($envvar)) {
	    $self->prepend($flag, Dpkg::BuildEnv::get($envvar), 'env');
	}
    }
}

=item $bf->load_maintainer_config()

Update flags based on maintainer directives stored in the environment. See
L<dpkg-buildflags(1)> for details.

=cut

sub load_maintainer_config {
    my $self = shift;

    foreach my $flag (keys %{$self->{flags}}) {
	my $envvar = 'DEB_' . $flag . '_MAINT_SET';
	if (Dpkg::BuildEnv::has($envvar)) {
	    $self->set($flag, Dpkg::BuildEnv::get($envvar), undef, 1);
	}
	$envvar = 'DEB_' . $flag . '_MAINT_STRIP';
	if (Dpkg::BuildEnv::has($envvar)) {
	    $self->strip($flag, Dpkg::BuildEnv::get($envvar), undef, 1);
	}
	$envvar = 'DEB_' . $flag . '_MAINT_APPEND';
	if (Dpkg::BuildEnv::has($envvar)) {
	    $self->append($flag, Dpkg::BuildEnv::get($envvar), undef, 1);
	}
	$envvar = 'DEB_' . $flag . '_MAINT_PREPEND';
	if (Dpkg::BuildEnv::has($envvar)) {
	    $self->prepend($flag, Dpkg::BuildEnv::get($envvar), undef, 1);
	}
    }
}


=item $bf->load_config()

Call successively load_system_config(), load_user_config(),
load_environment_config() and load_maintainer_config() to update the
default build flags defined by the vendor.

=cut

sub load_config {
    my $self = shift;

    $self->load_system_config();
    $self->load_user_config();
    $self->load_environment_config();
    $self->load_maintainer_config();
}

=item $bf->unset($flag)

Unset the build flag $flag, so that it will not be known anymore.

=cut

sub unset {
    my ($self, $flag) = @_;

    delete $self->{flags}->{$flag};
    delete $self->{origin}->{$flag};
    delete $self->{maintainer}->{$flag};
}

=item $bf->set($flag, $value, $source, $maint)

Update the build flag $flag with value $value and record its origin as
$source (if defined). Record it as maintainer modified if $maint is
defined and true.

=cut

sub set {
    my ($self, $flag, $value, $src, $maint) = @_;
    $self->{flags}->{$flag} = $value;
    $self->{origin}->{$flag} = $src if defined $src;
    $self->{maintainer}->{$flag} = $maint if $maint;
}

=item $bf->set_feature($area, $feature, $enabled)

Update the boolean state of whether a specific feature within a known
feature area has been enabled. The only currently known feature areas
are "future", "qa", "sanitize", "optimize", "hardening" and "reproducible".

=cut

sub set_feature {
    my ($self, $area, $feature, $enabled) = @_;
    $self->{features}{$area}{$feature} = $enabled;
}

=item $bf->get_feature($area, $feature)

Returns the value for the given feature within a known feature area.
This is relevant for builtin features where the feature has a ternary
state of true, false and undef, and where the latter cannot be retrieved
with use_feature().

=cut

sub get_feature {
    my ($self, $area, $feature) = @_;

    return if ! $self->has_features($area);
    return $self->{features}{$area}{$feature};
}

=item $bf->use_feature($area, $feature)

Returns true if the given feature within a known feature areas has been
enabled, and false otherwise.
The only currently recognized feature areas are "future", "qa", "sanitize",
"optimize", "hardening" and "reproducible".

=cut

sub use_feature {
    my ($self, $area, $feature) = @_;

    return 0 if ! $self->has_features($area);
    return 0 if ! $self->{features}{$area}{$feature};
    return 1;
}

=item $bf->set_builtin($area, $feature, $enabled)

Update the boolean state of whether a specific feature within a known
feature area is handled (even if only in some architectures) as a builtin
default by the compiler.

=cut

sub set_builtin {
    my ($self, $area, $feature, $enabled) = @_;
    $self->{builtins}{$area}{$feature} = $enabled;
}

=item $bf->get_builtins($area)

Return, for the given area, a hash with keys as feature names, and values
as booleans indicating whether the feature is handled as a builtin default
by the compiler or not. Only features that might be handled as builtins on
some architectures are returned as part of the hash. Missing features mean
they are currently never handled as builtins by the compiler.

=cut

sub get_builtins {
    my ($self, $area) = @_;
    return if ! exists $self->{builtins}{$area};
    return %{$self->{builtins}{$area}};
}

=item $bf->set_option_value($option, $value)

B<Private> method to set the value of a build option.
Do not use outside of the dpkg project.

=cut

sub set_option_value {
    my ($self, $option, $value) = @_;

    $self->{optvals}{$option} = $value;
}

=item $bf->get_option_value($option)

B<Private> method to get the value of a build option.
Do not use outside of the dpkg project.

=cut

sub get_option_value {
    my ($self, $option) = @_;

    return $self->{optvals}{$option};
}

=item $bf->strip($flag, $value, $source, $maint)

Update the build flag $flag by stripping the flags listed in $value and
record its origin as $source (if defined). Record it as maintainer modified
if $maint is defined and true.

=cut

sub strip {
    my ($self, $flag, $value, $src, $maint) = @_;

    my %strip = map { $_ => 1 } split /\s+/, $value;

    $self->{flags}->{$flag} = join q{ }, grep {
        ! exists $strip{$_}
    } split q{ }, $self->{flags}{$flag};
    $self->{origin}->{$flag} = $src if defined $src;
    $self->{maintainer}->{$flag} = $maint if $maint;
}

=item $bf->append($flag, $value, $source, $maint)

Append the options listed in $value to the current value of the flag $flag.
Record its origin as $source (if defined). Record it as maintainer modified
if $maint is defined and true.

=cut

sub append {
    my ($self, $flag, $value, $src, $maint) = @_;
    if (length($self->{flags}->{$flag})) {
        $self->{flags}->{$flag} .= " $value";
    } else {
        $self->{flags}->{$flag} = $value;
    }
    $self->{origin}->{$flag} = $src if defined $src;
    $self->{maintainer}->{$flag} = $maint if $maint;
}

=item $bf->prepend($flag, $value, $source, $maint)

Prepend the options listed in $value to the current value of the flag $flag.
Record its origin as $source (if defined). Record it as maintainer modified
if $maint is defined and true.

=cut

sub prepend {
    my ($self, $flag, $value, $src, $maint) = @_;
    if (length($self->{flags}->{$flag})) {
        $self->{flags}->{$flag} = "$value " . $self->{flags}->{$flag};
    } else {
        $self->{flags}->{$flag} = $value;
    }
    $self->{origin}->{$flag} = $src if defined $src;
    $self->{maintainer}->{$flag} = $maint if $maint;
}


=item $bf->update_from_conffile($file, $source)

Update the current build flags based on the configuration directives
contained in $file. See L<dpkg-buildflags(1)> for the format of the directives.

$source is the origin recorded for any build flag set or modified.

=cut

sub update_from_conffile {
    my ($self, $file, $src) = @_;
    local $_;

    return unless -e $file;
    open(my $conf_fh, '<', $file) or syserr(g_('cannot read %s'), $file);
    while (<$conf_fh>) {
        chomp;
        next if /^\s*#/; # Skip comments
        next if /^\s*$/; # Skip empty lines
        if (/^(append|prepend|set|strip)\s+(\S+)\s+(\S.*\S)\s*$/i) {
            my ($op, $flag, $value) = ($1, $2, $3);
            unless (exists $self->{flags}->{$flag}) {
                warning(g_('line %d of %s mentions unknown flag %s'), $., $file, $flag);
                $self->{flags}->{$flag} = '';
            }
            if (lc($op) eq 'set') {
                $self->set($flag, $value, $src);
            } elsif (lc($op) eq 'strip') {
                $self->strip($flag, $value, $src);
            } elsif (lc($op) eq 'append') {
                $self->append($flag, $value, $src);
            } elsif (lc($op) eq 'prepend') {
                $self->prepend($flag, $value, $src);
            }
        } else {
            warning(g_('line %d of %s is invalid, it has been ignored'), $., $file);
        }
    }
    close($conf_fh);
}

=item $bf->get($flag)

Return the value associated to the flag. It might be undef if the
flag doesn't exist.

=cut

sub get {
    my ($self, $key) = @_;
    return $self->{flags}{$key};
}

=item $bf->get_feature_areas()

Return the feature areas (i.e. the area values has_features will return
true for).

=cut

sub get_feature_areas {
    my $self = shift;

    return keys %{$self->{features}};
}

=item $bf->get_features($area)

Return, for the given area, a hash with keys as feature names, and values
as booleans indicating whether the feature is enabled or not.

=cut

sub get_features {
    my ($self, $area) = @_;
    return %{$self->{features}{$area}};
}

=item $bf->get_origin($flag)

Return the origin associated to the flag. It might be undef if the
flag doesn't exist.

=cut

sub get_origin {
    my ($self, $key) = @_;
    return $self->{origin}{$key};
}

=item $bf->is_maintainer_modified($flag)

Return true if the flag is modified by the maintainer.

=cut

sub is_maintainer_modified {
    my ($self, $key) = @_;
    return $self->{maintainer}{$key};
}

=item $bf->has_features($area)

Returns true if the given area of features is known, and false otherwise.
The only currently recognized feature areas are "future", "qa", "sanitize",
"optimize", "hardening" and "reproducible".

=cut

sub has_features {
    my ($self, $area) = @_;
    return exists $self->{features}{$area};
}

=item $bf->has($option)

Returns a boolean indicating whether the flags exists in the object.

=cut

sub has {
    my ($self, $key) = @_;
    return exists $self->{flags}{$key};
}

=item @flags = $bf->list()

Returns the list of flags stored in the object.

=cut

sub list {
    my $self = shift;
    my @list = sort keys %{$self->{flags}};
    return @list;
}

=back

=head1 CHANGES

=head2 Version 1.06 (dpkg 1.21.15)

New method: $bf->get_feature().

=head2 Version 1.05 (dpkg 1.21.14)

New option: 'vendor_defaults' in new().

New methods: $bf->load_vendor_defaults(), $bf->use_feature(),
$bf->set_builtin(), $bf->get_builtins().

=head2 Version 1.04 (dpkg 1.20.0)

New method: $bf->unset().

=head2 Version 1.03 (dpkg 1.16.5)

New method: $bf->get_feature_areas() to list possible values for
$bf->get_features.

New method $bf->is_maintainer_modified() and new optional parameter to
$bf->set(), $bf->append(), $bf->prepend(), $bf->strip().

=head2 Version 1.02 (dpkg 1.16.2)

New methods: $bf->get_features(), $bf->has_features(), $bf->set_feature().

=head2 Version 1.01 (dpkg 1.16.1)

New method: $bf->prepend() very similar to append(). Implement support of
the prepend operation everywhere.

New method: $bf->load_maintainer_config() that update the build flags
based on the package maintainer directives.

=head2 Version 1.00 (dpkg 1.15.7)

Mark the module as public.

=cut

1;
