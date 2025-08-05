# Copyright © 2009-2011 Raphaël Hertzog <hertzog@debian.org>
# Copyright © 2009-2024 Guillem Jover <guillem@debian.org>
#
# Hardening build flags handling derived from work of:
# Copyright © 2009-2011 Kees Cook <kees@debian.org>
# Copyright © 2007-2008 Canonical, Ltd.
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

Dpkg::Vendor::Debian - Debian vendor class

=head1 DESCRIPTION

This vendor class customizes the behavior of dpkg scripts for Debian
specific behavior and policies.

B<Note>: This is a private module, its API can change at any time.

=cut

package Dpkg::Vendor::Debian 0.01;

use strict;
use warnings;

use List::Util qw(any none);

use Dpkg;
use Dpkg::Gettext;
use Dpkg::ErrorHandling;
use Dpkg::Control::Types;

use parent qw(Dpkg::Vendor::Default);

sub run_hook {
    my ($self, $hook, @params) = @_;

    if ($hook eq 'package-keyrings') {
        return ('/usr/share/keyrings/debian-keyring.gpg',
                '/usr/share/keyrings/debian-tag2upload.pgp',
                '/usr/share/keyrings/debian-nonupload.gpg',
                '/usr/share/keyrings/debian-maintainers.gpg');
    } elsif ($hook eq 'archive-keyrings') {
        return ('/usr/share/keyrings/debian-archive-keyring.gpg');
    } elsif ($hook eq 'archive-keyrings-historic') {
        return ('/usr/share/keyrings/debian-archive-removed-keys.gpg');
    } elsif ($hook eq 'builtin-build-depends') {
        return qw(build-essential:native);
    } elsif ($hook eq 'builtin-build-conflicts') {
        return ();
    } elsif ($hook eq 'register-custom-fields') {
    } elsif ($hook eq 'extend-patch-header') {
        my ($textref, $ch_info) = @params;
	if ($ch_info->{'Closes'}) {
	    foreach my $bug (split(/\s+/, $ch_info->{'Closes'})) {
		$$textref .= "Bug-Debian: https://bugs.debian.org/$bug\n";
	    }
	}

	# XXX: Layer violation...
	require Dpkg::Vendor::Ubuntu;
	my $b = Dpkg::Vendor::Ubuntu::find_launchpad_closes($ch_info->{'Changes'});
	foreach my $bug (@$b) {
	    $$textref .= "Bug-Ubuntu: https://bugs.launchpad.net/bugs/$bug\n";
	}
    } elsif ($hook eq 'update-buildflags') {
        $self->set_build_features(@params);
        $self->add_build_flags(@params);
    } elsif ($hook eq 'builtin-system-build-paths') {
        return qw(/build/);
    } elsif ($hook eq 'build-tainted-by') {
        return $self->_build_tainted_by();
    } elsif ($hook eq 'sanitize-environment') {
        # Reset umask to a sane default.
        umask 0022;
        # Reset locale to a sane default.
        #
        # We ignore the LANGUAGE GNU extension, as that only affects
        # LC_MESSAGES which will use LC_CTYPE for its codeset. We need to
        # move the high priority LC_ALL catch-all into the low-priority
        # LANG catch-all so that we can override LC_* variables, and remove
        # any existing LC_* variables which would have been ignored anyway,
        # and would now take precedence over LANG.
        if (length $ENV{LC_ALL}) {
            $ENV{LANG} = delete $ENV{LC_ALL};
            foreach my $lc (grep { m/^LC_/ } keys %ENV) {
                delete $ENV{$lc};
            }
        }
        $ENV{LC_COLLATE} = 'C.UTF-8';
        $ENV{LC_CTYPE} = 'C.UTF-8';
    } elsif ($hook eq 'backport-version-regex') {
        return qr/~(bpo|deb)/;
    } else {
        return $self->SUPER::run_hook($hook, @params);
    }
}

sub init_build_features {
    my ($self, $use_feature, $builtin_feature) = @_;
}

sub set_build_features {
    my ($self, $flags) = @_;

    # Default feature states.
    my %use_feature = (
        future => {
            # XXX: Should start a deprecation cycle at some point.
            lfs => 0,
        },
        abi => {
            # XXX: This is set to undef so that we can handle the alias from
            # the future feature area.
            lfs => undef,
            # XXX: This is set to undef to handle mask on the default setting.
            time64 => undef,
        },
        qa => {
            bug => undef,
            'bug-implicit-func' => undef,
            canary => 0,
        },
        reproducible => {
            timeless => 1,
            fixfilepath => 1,
            fixdebugpath => 1,
        },
        optimize => {
            lto => 0,
        },
        sanitize => {
            address => 0,
            thread => 0,
            leak => 0,
            undefined => 0,
        },
        hardening => {
            # XXX: This is set to undef so that we can cope with the brokenness
            # of gcc managing this feature builtin.
            pie => undef,
            stackprotector => 1,
            stackprotectorstrong => 1,
            stackclash => 1,
            fortify => 1,
            format => 1,
            relro => 1,
            bindnow => 0,
            branch => 1,
        },
    );

    my %builtin_feature = (
        abi => {
            lfs => 0,
            time64 => 0,
        },
        hardening => {
            pie => 1,
        },
    );

    require Dpkg::Arch;

    my $arch = Dpkg::Arch::get_host_arch();
    my ($abi, $libc, $os, $cpu) = Dpkg::Arch::debarch_to_debtuple($arch);
    my ($abi_bits, $abi_endian) = Dpkg::Arch::debarch_to_abiattrs($arch);

    unless (defined $abi and defined $libc and defined $os and defined $cpu) {
        warning(g_("unknown host architecture '%s'"), $arch);
        ($abi, $os, $cpu) = ('', '', '');
    }
    unless (defined $abi_bits and defined $abi_endian) {
        warning(g_("unknown abi attributes for architecture '%s'"), $arch);
        ($abi_bits, $abi_endian) = (0, 'unknown');
    }

    # Mask builtin features that are not enabled by default in the compiler.
    my %builtin_pie_arch = map { $_ => 1 } qw(
        amd64
        arm64
        armel
        armhf
        hurd-amd64
        hurd-i386
        i386
        kfreebsd-amd64
        kfreebsd-i386
        loong64
        mips
        mips64
        mips64el
        mips64r6
        mips64r6el
        mipsel
        mipsn32
        mipsn32el
        mipsn32r6
        mipsn32r6el
        mipsr6
        mipsr6el
        powerpc
        ppc64
        ppc64el
        riscv64
        s390x
        sparc
        sparc64
    );
    if (not exists $builtin_pie_arch{$arch}) {
        $builtin_feature{hardening}{pie} = 0;
    }

    if ($abi_bits != 32) {
        $builtin_feature{abi}{lfs} = 1;
    }

    # On glibc, new ports default to time64, old ports currently default
    # to time32, so we track the latter as that is a list that is not
    # going to grow further, and might shrink.
    # On musl libc based systems all ports use time64.
    my %time32_arch = map { $_ => 1 } qw(
        arm
        armeb
        armel
        armhf
        hppa
        i386
        hurd-i386
        kfreebsd-i386
        m68k
        mips
        mipsel
        mipsn32
        mipsn32el
        mipsn32r6
        mipsn32r6el
        mipsr6
        mipsr6el
        nios2
        powerpc
        powerpcel
        powerpcspe
        s390
        sh3
        sh3eb
        sh4
        sh4eb
        sparc
    );
    if ($abi_bits != 32 or
        not exists $time32_arch{$arch} or
        $libc eq 'musl') {
        $builtin_feature{abi}{time64} = 1;
    }

    $self->init_build_features(\%use_feature, \%builtin_feature);

    ## Setup

    require Dpkg::BuildOptions;

    # Adjust features based on user or maintainer's desires.
    my $opts_build = Dpkg::BuildOptions->new(envvar => 'DEB_BUILD_OPTIONS');
    my $opts_maint = Dpkg::BuildOptions->new(envvar => 'DEB_BUILD_MAINT_OPTIONS');

    foreach my $area (sort keys %use_feature) {
        $opts_build->parse_features($area, $use_feature{$area});
        $opts_maint->parse_features($area, $use_feature{$area});
    }

    ## Area: abi

    if (any { $arch eq $_ } qw(hurd-i386 kfreebsd-i386)) {
        # Mask time64 on hurd-i386 and kfreebsd-i386, as their kernel lacks
        # support for that arch and it will not be implemented.
        $use_feature{abi}{time64} = 0;
    } elsif (not defined $use_feature{abi}{time64}) {
        # If the user has not requested a specific setting, by default only
        # enable time64 everywhere except for i386, where we preserve it for
        # binary backwards compatibility.
        if ($arch eq 'i386') {
            $use_feature{abi}{time64} = 0;
        } else {
            $use_feature{abi}{time64} = 1;
        }
    }

    # In Debian gcc enables time64 (and lfs) for the following architectures
    # by injecting pre-processor flags, though the libc ABI has not changed.
    if (any { $arch eq $_ } qw(armel armhf hppa m68k mips mipsel powerpc sh4)) {
        $flags->set_option_value('cc-abi-time64', 1);
    } else {
        $flags->set_option_value('cc-abi-time64', 0);
    }

    if ($use_feature{abi}{time64} && ! $builtin_feature{abi}{time64}) {
        # On glibc 64-bit time_t support requires LFS.
        $use_feature{abi}{lfs} = 1 if $libc eq 'gnu';
    }

    # XXX: Handle lfs alias from future abi feature area.
    $use_feature{abi}{lfs} //= $use_feature{future}{lfs};
    # XXX: Once the feature is set in the abi area, we always override the
    # one in the future area.
    $use_feature{future}{lfs} = $use_feature{abi}{lfs};

    ## Area: qa

    # For time64 we require -Werror=implicit-function-declaration, to avoid
    # linking against the wrong symbol. Instead of enabling this conditionally
    # on time64 being enabled, do it unconditionally so that the effects are
    # uniform and visible on all architectures. Unless it has been set
    # explicitly.
    $use_feature{qa}{'bug-implicit-func'} //= $use_feature{qa}{bug} // 1;

    $use_feature{qa}{bug} //= 0;

    ## Area: reproducible

    # Mask features that might have an unsafe usage.
    if ($use_feature{reproducible}{fixfilepath} or
        $use_feature{reproducible}{fixdebugpath}) {
        require Cwd;

        my $build_path =$ENV{DEB_BUILD_PATH} || Cwd::getcwd();

        $flags->set_option_value('build-path', $build_path);

        # If we have any unsafe character in the path, disable the flag,
        # so that we do not need to worry about escaping the characters
        # on output.
        if ($build_path =~ m/[^-+:.0-9a-zA-Z~\/_]/) {
            $use_feature{reproducible}{fixfilepath} = 0;
            $use_feature{reproducible}{fixdebugpath} = 0;
        }
    }

    ## Area: optimize

    if ($opts_build->has('noopt')) {
        $flags->set_option_value('optimize-level', 0);
    } else {
        $flags->set_option_value('optimize-level', 2);
    }

    ## Area: sanitize

    # Handle logical feature interactions.
    if ($use_feature{sanitize}{address} and $use_feature{sanitize}{thread}) {
        # Disable the thread sanitizer when the address one is active, they
        # are mutually incompatible.
        $use_feature{sanitize}{thread} = 0;
    }
    if ($use_feature{sanitize}{address} or $use_feature{sanitize}{thread}) {
        # Disable leak sanitizer, it is implied by the address or thread ones.
        $use_feature{sanitize}{leak} = 0;
    }

    ## Area: hardening

    # Mask features that are not available on certain architectures.
    if (none { $os eq $_ } qw(linux kfreebsd hurd) or
        any { $cpu eq $_ } qw(alpha hppa ia64)) {
	# Disabled on non-(linux/kfreebsd/hurd).
        # Disabled on alpha, hppa, ia64.
	$use_feature{hardening}{pie} = 0;
    }
    if (any { $cpu eq $_ } qw(ia64 alpha hppa nios2) or $arch eq 'arm') {
	# Stack protector disabled on ia64, alpha, hppa, nios2.
	#   "warning: -fstack-protector not supported for this target"
	# Stack protector disabled on arm (ok on armel).
	#   compiler supports it incorrectly (leads to SEGV)
	$use_feature{hardening}{stackprotector} = 0;
    }
    if (none { $arch eq $_ } qw(amd64 arm64 armhf armel)) {
        # Stack clash protector only available on amd64 and arm.
        $use_feature{hardening}{stackclash} = 0;
    }
    if (any { $cpu eq $_ } qw(ia64 hppa)) {
	# relro not implemented on ia64, hppa.
	$use_feature{hardening}{relro} = 0;
    }
    if (none { $cpu eq $_ } qw(amd64 arm64)) {
        # On amd64 use -fcf-protection.
        # On arm64 use -mbranch-protection=standard.
        $use_feature{hardening}{branch} = 0;
    }
    $flags->set_option_value('hardening-branch-cpu', $cpu);

    # Mask features that might be influenced by other flags.
    if ($flags->get_option_value('optimize-level') == 0) {
      # glibc 2.16 and later warn when using -O0 and _FORTIFY_SOURCE.
      $use_feature{hardening}{fortify} = 0;
    }
    $flags->set_option_value('fortify-level', 2);

    # Handle logical feature interactions.
    if ($use_feature{hardening}{relro} == 0) {
	# Disable bindnow if relro is not enabled, since it has no
	# hardening ability without relro and may incur load penalties.
	$use_feature{hardening}{bindnow} = 0;
    }
    if ($use_feature{hardening}{stackprotector} == 0) {
	# Disable stackprotectorstrong if stackprotector is disabled.
	$use_feature{hardening}{stackprotectorstrong} = 0;
    }

    ## Commit

    # Set used features to their builtin setting if unset.
    foreach my $area (sort keys %builtin_feature) {
        while (my ($feature, $enabled) = each %{$builtin_feature{$area}}) {
            $flags->set_builtin($area, $feature, $enabled);
        }
    }

    # Store the feature usage.
    foreach my $area (sort keys %use_feature) {
        while (my ($feature, $enabled) = each %{$use_feature{$area}}) {
            $flags->set_feature($area, $feature, $enabled);
        }
    }
}

sub add_build_flags {
    my ($self, $flags) = @_;

    ## Global default flags

    my @compile_flags = qw(
        CFLAGS
        CXXFLAGS
        OBJCFLAGS
        OBJCXXFLAGS
        FFLAGS
        FCFLAGS
    );

    my $default_flags;
    my $default_d_flags;

    my $optimize_level = $flags->get_option_value('optimize-level');
    $default_flags = "-g -O$optimize_level";
    if ($optimize_level == 0) {
        $default_d_flags = '-fdebug';
    } else {
        $default_d_flags = '-frelease';
    }

    $flags->append($_, $default_flags) foreach @compile_flags;
    $flags->append('DFLAGS', $default_d_flags);

    ## Area: abi

    my %abi_builtins = $flags->get_builtins('abi');
    my $cc_abi_time64 = $flags->get_option_value('cc-abi-time64');

    if ($flags->use_feature('abi', 'lfs') && ! $abi_builtins{lfs}) {
        $flags->append('CPPFLAGS',
                       '-D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64');
    } elsif (! $flags->use_feature('abi', 'lfs') &&
             ! $abi_builtins{lfs} && $cc_abi_time64) {
        $flags->append('CPPFLAGS',
                       '-U_LARGEFILE_SOURCE -U_FILE_OFFSET_BITS');
    }

    if ($flags->use_feature('abi', 'time64') && ! $abi_builtins{time64}) {
        $flags->append('CPPFLAGS', '-D_TIME_BITS=64');
    } elsif (! $flags->use_feature('abi', 'time64') &&
             ! $abi_builtins{time64} && $cc_abi_time64) {
        $flags->append('CPPFLAGS', '-U_TIME_BITS');
    }

    ## Area: qa

    # Warnings that detect actual bugs.
    if ($flags->use_feature('qa', 'bug-implicit-func')) {
        $flags->append('CFLAGS', '-Werror=implicit-function-declaration');
    } else {
        $flags->append('CFLAGS', '-Wno-error=implicit-function-declaration');
    }
    if ($flags->use_feature('qa', 'bug')) {
        # C/C++ flags
        my @cfamilyflags = qw(
            array-bounds
            clobbered
            volatile-register-var
        );
        foreach my $warnflag (@cfamilyflags) {
            $flags->append('CFLAGS', "-Werror=$warnflag");
            $flags->append('CXXFLAGS', "-Werror=$warnflag");
        }
    }

    # Inject dummy canary options to detect issues with build flag propagation.
    if ($flags->use_feature('qa', 'canary')) {
        require Digest::MD5;
        my $id = Digest::MD5::md5_hex(int rand 4096);

        foreach my $flag (qw(CPPFLAGS CFLAGS OBJCFLAGS CXXFLAGS OBJCXXFLAGS)) {
            $flags->append($flag, "-D__DEB_CANARY_${flag}_${id}__");
        }
        $flags->append('LDFLAGS', "-Wl,-z,deb-canary-${id}");
    }

    ## Area: reproducible

    # Warn when the __TIME__, __DATE__ and __TIMESTAMP__ macros are used.
    if ($flags->use_feature('reproducible', 'timeless')) {
       $flags->append('CPPFLAGS', '-Wdate-time');
    }

    # Avoid storing the build path in the binaries.
    if ($flags->use_feature('reproducible', 'fixfilepath') or
        $flags->use_feature('reproducible', 'fixdebugpath')) {
        my $build_path = $flags->get_option_value('build-path');
        my $map;

        # -ffile-prefix-map is a superset of -fdebug-prefix-map, prefer it
        # if both are set.
        if ($flags->use_feature('reproducible', 'fixfilepath')) {
            $map = '-ffile-prefix-map=' . $build_path . '=.';
        } else {
            $map = '-fdebug-prefix-map=' . $build_path . '=.';
        }

        $flags->append($_, $map) foreach @compile_flags;
    }

    ## Area: optimize

    if ($flags->use_feature('optimize', 'lto')) {
        my $flag = '-flto=auto -ffat-lto-objects';
        $flags->append($_, $flag) foreach (@compile_flags, 'LDFLAGS');
    }

    ## Area: sanitize

    if ($flags->use_feature('sanitize', 'address')) {
        my $flag = '-fsanitize=address -fno-omit-frame-pointer';
        $flags->append('CFLAGS', $flag);
        $flags->append('CXXFLAGS', $flag);
        $flags->append('LDFLAGS', '-fsanitize=address');
    }

    if ($flags->use_feature('sanitize', 'thread')) {
        my $flag = '-fsanitize=thread';
        $flags->append('CFLAGS', $flag);
        $flags->append('CXXFLAGS', $flag);
        $flags->append('LDFLAGS', $flag);
    }

    if ($flags->use_feature('sanitize', 'leak')) {
        $flags->append('LDFLAGS', '-fsanitize=leak');
    }

    if ($flags->use_feature('sanitize', 'undefined')) {
        my $flag = '-fsanitize=undefined';
        $flags->append('CFLAGS', $flag);
        $flags->append('CXXFLAGS', $flag);
        $flags->append('LDFLAGS', $flag);
    }

    ## Area: hardening

    # PIE
    my $use_pie = $flags->get_feature('hardening', 'pie');
    my %hardening_builtins = $flags->get_builtins('hardening');
    if (defined $use_pie && $use_pie && ! $hardening_builtins{pie}) {
	my $flag = "-specs=$Dpkg::DATADIR/pie-compile.specs";
        $flags->append($_, $flag) foreach @compile_flags;
	$flags->append('LDFLAGS', "-specs=$Dpkg::DATADIR/pie-link.specs");
    } elsif (defined $use_pie && ! $use_pie && $hardening_builtins{pie}) {
	my $flag = "-specs=$Dpkg::DATADIR/no-pie-compile.specs";
        $flags->append($_, $flag) foreach @compile_flags;
	$flags->append('LDFLAGS', "-specs=$Dpkg::DATADIR/no-pie-link.specs");
    }

    # Stack protector
    if ($flags->use_feature('hardening', 'stackprotectorstrong')) {
	my $flag = '-fstack-protector-strong';
        $flags->append($_, $flag) foreach @compile_flags;
    } elsif ($flags->use_feature('hardening', 'stackprotector')) {
	my $flag = '-fstack-protector --param=ssp-buffer-size=4';
        $flags->append($_, $flag) foreach @compile_flags;
    }

    # Stack clash
    if ($flags->use_feature('hardening', 'stackclash')) {
        my $flag = '-fstack-clash-protection';
        $flags->append($_, $flag) foreach @compile_flags;
    }

    # Fortify Source
    if ($flags->use_feature('hardening', 'fortify')) {
        my $fortify_level = $flags->get_option_value('fortify-level');
        $flags->append('CPPFLAGS', "-D_FORTIFY_SOURCE=$fortify_level");
    }

    # Format Security
    if ($flags->use_feature('hardening', 'format')) {
	my $flag = '-Wformat -Werror=format-security';
	$flags->append('CFLAGS', $flag);
	$flags->append('CXXFLAGS', $flag);
	$flags->append('OBJCFLAGS', $flag);
	$flags->append('OBJCXXFLAGS', $flag);
    }

    # Read-only Relocations
    if ($flags->use_feature('hardening', 'relro')) {
	$flags->append('LDFLAGS', '-Wl,-z,relro');
    }

    # Bindnow
    if ($flags->use_feature('hardening', 'bindnow')) {
	$flags->append('LDFLAGS', '-Wl,-z,now');
    }

    # Branch protection
    if ($flags->use_feature('hardening', 'branch')) {
        my $cpu = $flags->get_option_value('hardening-branch-cpu');
        my $flag;
        if ($cpu eq 'arm64') {
            $flag = '-mbranch-protection=standard';
        } elsif ($cpu eq 'amd64') {
            $flag = '-fcf-protection';
        }
        # The following should always be true on Debian, but it might not
        # be on derivatives.
        if (defined $flag) {
            $flags->append($_, $flag) foreach @compile_flags;
        }
    }

    # XXX: Handle *_FOR_BUILD flags here until we can properly initialize them.
    require Dpkg::Arch;

    my $host_arch = Dpkg::Arch::get_host_arch();
    my $build_arch = Dpkg::Arch::get_build_arch();

    if ($host_arch eq $build_arch) {
        foreach my $flag ($flags->list()) {
            next if $flag =~ m/_FOR_BUILD$/;
            my $value = $flags->get($flag);
            $flags->append($flag . '_FOR_BUILD', $value);
        }
    } else {
        $flags->append($_ . '_FOR_BUILD', $default_flags) foreach @compile_flags;
        $flags->append('DFLAGS_FOR_BUILD', $default_d_flags);
    }
}

sub _build_tainted_by {
    my $self = shift;
    my %tainted;

    require File::Find;
    my %usr_local_types = (
        configs => [ qw(etc) ],
        includes => [ qw(include) ],
        programs => [ qw(bin sbin) ],
        libraries => [ qw(lib) ],
    );
    foreach my $type (keys %usr_local_types) {
        File::Find::find({
            wanted => sub { $tainted{"usr-local-has-$type"} = 1 if -f },
            no_chdir => 1,
        }, grep { -d } map { "/usr/local/$_" } @{$usr_local_types{$type}});
    }

    my @tainted = sort keys %tainted;
    return @tainted;
}

=head1 CHANGES

=head2 Version 0.xx

This is a private module.

=cut

1;
