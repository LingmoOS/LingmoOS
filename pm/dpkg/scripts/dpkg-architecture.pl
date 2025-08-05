#!/usr/bin/perl
#
# dpkg-architecture
#
# Copyright © 1999-2001 Marcus Brinkmann <brinkmd@debian.org>
# Copyright © 2004-2005 Scott James Remnant <scott@netsplit.com>,
# Copyright © 2006-2014 Guillem Jover <guillem@debian.org>
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

use strict;
use warnings;

use Dpkg ();
use Dpkg::Gettext;
use Dpkg::Getopt;
use Dpkg::ErrorHandling;
use Dpkg::Arch qw(:getters :mappers debarch_eq debarch_is);

textdomain('dpkg-dev');

sub version {
    printf g_("Debian %s version %s.\n"), $Dpkg::PROGNAME, $Dpkg::PROGVERSION;

    printf g_('
This is free software; see the GNU General Public License version 2 or
later for copying conditions. There is NO warranty.
');
}

sub usage {
    printf g_(
'Usage: %s [<option>...] [<command>]')
    . "\n\n" . g_(
'Commands:
  -l, --list                list variables (default).
  -L, --list-known          list valid architectures (matching some criteria).
  -e, --equal <arch>        compare with host Debian architecture.
  -i, --is <arch-wildcard>  match against host Debian architecture.
  -q, --query <variable>    prints only the value of <variable>.
  -s, --print-set           print command to set environment variables.
  -u, --print-unset         print command to unset environment variables.
  -c, --command <command>   set environment and run the command in it.
  -?, --help                show this help message.
      --version             show the version.')
    . "\n\n" . g_(
'Options:
  -a, --host-arch <arch>    set host Debian architecture.
  -t, --host-type <type>    set host GNU system type.
  -A, --target-arch <arch>  set target Debian architecture.
  -T, --target-type <type>  set target GNU system type.
  -W, --match-wildcard <arch-wildcard>
                            restrict architecture list matching <arch-wildcard>.
  -B, --match-bits <arch-bits>
                            restrict architecture list matching <arch-bits>.
  -E, --match-endian <arch-endian>
                            restrict architecture list matching <arch-endian>.
      --print-format <format>
                            use <format> for --print-set and --print-unset,
                              allowed values: shell (default), make.
  -f, --force               force flag (override variables set in environment).')
    . "\n", $Dpkg::PROGNAME;
}

sub check_arch_coherency
{
    my ($arch, $gnu_type) = @_;

    if ($arch ne '' && $gnu_type eq '') {
        $gnu_type = debarch_to_gnutriplet($arch);
        error(g_('unknown Debian architecture %s, you must specify ' .
                 'GNU system type, too'), $arch)
            unless defined $gnu_type;
    }

    if ($gnu_type ne '' && $arch eq '') {
        $arch = gnutriplet_to_debarch($gnu_type);
        error(g_('unknown GNU system type %s, you must specify ' .
                 'Debian architecture, too'), $gnu_type)
            unless defined $arch;
    }

    if ($gnu_type ne '' && $arch ne '') {
        my $dfl_gnu_type = debarch_to_gnutriplet($arch);
        error(g_('unknown default GNU system type for Debian architecture %s'),
              $arch)
            unless defined $dfl_gnu_type;
        warning(g_('default GNU system type %s for Debian arch %s does not ' .
                   'match specified GNU system type %s'), $dfl_gnu_type,
                $arch, $gnu_type)
            if $dfl_gnu_type ne $gnu_type;
    }

    return ($arch, $gnu_type);
}

use constant {
    INFO_BUILD_ARCH_NAME        => 0b00001,
    INFO_BUILD_ARCH_TUPLE       => 0b00010,
    INFO_BUILD_ARCH_ATTR        => 0b00100,
    INFO_BUILD_MULTIARCH        => 0b01000,
    INFO_BUILD_GNU_TUPLE        => 0b10000,

    INFO_HOST_ARCH_NAME         => 0b0000100000,
    INFO_HOST_ARCH_TUPLE        => 0b0001000000,
    INFO_HOST_ARCH_ATTR         => 0b0010000000,
    INFO_HOST_MULTIARCH         => 0b0100000000,
    INFO_HOST_GNU_TUPLE         => 0b1000000000,

    INFO_TARGET_ARCH_NAME       => 0b000010000000000,
    INFO_TARGET_ARCH_TUPLE      => 0b000100000000000,
    INFO_TARGET_ARCH_ATTR       => 0b001000000000000,
    INFO_TARGET_MULTIARCH       => 0b010000000000000,
    INFO_TARGET_GNU_TUPLE       => 0b100000000000000,
};

my %arch_vars = (
    DEB_BUILD_ARCH          => INFO_BUILD_ARCH_NAME,
    DEB_BUILD_ARCH_ABI      => INFO_BUILD_ARCH_NAME | INFO_BUILD_ARCH_TUPLE,
    DEB_BUILD_ARCH_LIBC     => INFO_BUILD_ARCH_NAME | INFO_BUILD_ARCH_TUPLE,
    DEB_BUILD_ARCH_OS       => INFO_BUILD_ARCH_NAME | INFO_BUILD_ARCH_TUPLE,
    DEB_BUILD_ARCH_CPU      => INFO_BUILD_ARCH_NAME | INFO_BUILD_ARCH_TUPLE,
    DEB_BUILD_ARCH_BITS     => INFO_BUILD_ARCH_NAME | INFO_BUILD_ARCH_ATTR,
    DEB_BUILD_ARCH_ENDIAN   => INFO_BUILD_ARCH_NAME | INFO_BUILD_ARCH_ATTR,
    DEB_BUILD_MULTIARCH     => INFO_BUILD_ARCH_NAME | INFO_BUILD_MULTIARCH,
    DEB_BUILD_GNU_CPU       => INFO_BUILD_ARCH_NAME | INFO_BUILD_GNU_TUPLE,
    DEB_BUILD_GNU_SYSTEM    => INFO_BUILD_ARCH_NAME | INFO_BUILD_GNU_TUPLE,
    DEB_BUILD_GNU_TYPE      => INFO_BUILD_ARCH_NAME | INFO_BUILD_GNU_TUPLE,
    DEB_HOST_ARCH           => INFO_HOST_ARCH_NAME,
    DEB_HOST_ARCH_ABI       => INFO_HOST_ARCH_NAME | INFO_HOST_ARCH_TUPLE,
    DEB_HOST_ARCH_LIBC      => INFO_HOST_ARCH_NAME | INFO_HOST_ARCH_TUPLE,
    DEB_HOST_ARCH_OS        => INFO_HOST_ARCH_NAME | INFO_HOST_ARCH_TUPLE,
    DEB_HOST_ARCH_CPU       => INFO_HOST_ARCH_NAME | INFO_HOST_ARCH_TUPLE,
    DEB_HOST_ARCH_BITS      => INFO_HOST_ARCH_NAME | INFO_HOST_ARCH_ATTR,
    DEB_HOST_ARCH_ENDIAN    => INFO_HOST_ARCH_NAME | INFO_HOST_ARCH_ATTR,
    DEB_HOST_MULTIARCH      => INFO_HOST_ARCH_NAME | INFO_HOST_MULTIARCH,
    DEB_HOST_GNU_CPU        => INFO_HOST_ARCH_NAME | INFO_HOST_GNU_TUPLE,
    DEB_HOST_GNU_SYSTEM     => INFO_HOST_ARCH_NAME | INFO_HOST_GNU_TUPLE,
    DEB_HOST_GNU_TYPE       => INFO_HOST_ARCH_NAME | INFO_HOST_GNU_TUPLE,
    DEB_TARGET_ARCH         => INFO_TARGET_ARCH_NAME,
    DEB_TARGET_ARCH_ABI     => INFO_TARGET_ARCH_NAME | INFO_TARGET_ARCH_TUPLE,
    DEB_TARGET_ARCH_LIBC    => INFO_TARGET_ARCH_NAME | INFO_TARGET_ARCH_TUPLE,
    DEB_TARGET_ARCH_OS      => INFO_TARGET_ARCH_NAME | INFO_TARGET_ARCH_TUPLE,
    DEB_TARGET_ARCH_CPU     => INFO_TARGET_ARCH_NAME | INFO_TARGET_ARCH_TUPLE,
    DEB_TARGET_ARCH_BITS    => INFO_TARGET_ARCH_NAME | INFO_TARGET_ARCH_ATTR,
    DEB_TARGET_ARCH_ENDIAN  => INFO_TARGET_ARCH_NAME | INFO_TARGET_ARCH_ATTR,
    DEB_TARGET_MULTIARCH    => INFO_TARGET_ARCH_NAME | INFO_TARGET_MULTIARCH,
    DEB_TARGET_GNU_CPU      => INFO_TARGET_ARCH_NAME | INFO_TARGET_GNU_TUPLE,
    DEB_TARGET_GNU_SYSTEM   => INFO_TARGET_ARCH_NAME | INFO_TARGET_GNU_TUPLE,
    DEB_TARGET_GNU_TYPE     => INFO_TARGET_ARCH_NAME | INFO_TARGET_GNU_TUPLE,
);

my %known_print_format = map { $_ => 1 } qw(shell make);
my $print_format = 'shell';

my %req_vars = %arch_vars;
my $req_info = 0;
my $req_host_arch = '';
my $req_host_gnu_type = '';
my $req_target_arch = '';
my $req_target_gnu_type = '';
my $req_eq_arch = '';
my $req_is_arch = '';
my $req_match_wildcard = '';
my $req_match_bits = '';
my $req_match_endian = '';
my $req_variable_to_print;
my $action = 'list';
my $force = 0;

sub action_needs {
    my $bits = shift;
    return (($req_info & $bits) == $bits);
}

@ARGV = normalize_options(args => \@ARGV, delim => '-c');

while (@ARGV) {
    my $arg = shift;

    if ($arg eq '-a' or $arg eq '--host-arch') {
	$req_host_arch = shift;
    } elsif ($arg eq '-t' or $arg eq '--host-type') {
	$req_host_gnu_type = shift;
    } elsif ($arg eq '-A' or $arg eq '--target-arch') {
	$req_target_arch = shift;
    } elsif ($arg eq '-T' or $arg eq '--target-type') {
	$req_target_gnu_type = shift;
    } elsif ($arg eq '-W' or $arg eq '--match-wildcard') {
	$req_match_wildcard = shift;
    } elsif ($arg eq '-B' or $arg eq '--match-bits') {
	$req_match_bits = shift;
    } elsif ($arg eq '-E' or $arg eq '--match-endian') {
	$req_match_endian = shift;
    } elsif ($arg eq '-e' or $arg eq '--equal') {
	$req_eq_arch = shift;
        %req_vars = %arch_vars{DEB_HOST_ARCH};
	$action = 'equal';
    } elsif ($arg eq '-i' or $arg eq '--is') {
	$req_is_arch = shift;
        %req_vars = %arch_vars{DEB_HOST_ARCH};
	$action = 'is';
    } elsif ($arg eq '-u' or $arg eq '--print-unset') {
        %req_vars = ();
	$action = 'print-unset';
    } elsif ($arg eq '-l' or $arg eq '--list') {
	$action = 'list';
    } elsif ($arg eq '-s' or $arg eq '--print-set') {
        %req_vars = %arch_vars;
	$action = 'print-set';
    } elsif ($arg eq '--print-format') {
        $print_format = shift;
        error(g_('%s is not a supported print format'), $print_format)
            unless exists $known_print_format{$print_format};
    } elsif ($arg eq '-f' or $arg eq '--force') {
        $force = 1;
    } elsif ($arg eq '-q' or $arg eq '--query') {
	my $varname = shift;
	error(g_('%s is not a supported variable name'), $varname)
	    unless (exists $arch_vars{$varname});
	$req_variable_to_print = "$varname";
        %req_vars = %arch_vars{$varname};
        $action = 'query';
    } elsif ($arg eq '-c' or $arg eq '--command') {
        $action = 'command';
        last;
    } elsif ($arg eq '-L' or $arg eq '--list-known') {
        %req_vars = ();
        $action = 'list-known';
    } elsif ($arg eq '-?' or $arg eq '--help') {
        usage();
       exit 0;
    } elsif ($arg eq '--version') {
        version();
       exit 0;
    } else {
        usageerr(g_("unknown option '%s'"), $arg);
    }
}

my %v;

# Initialize variables from environment and information to gather.
foreach my $k (keys %req_vars) {
    if (length $ENV{$k} && ! $force) {
        $v{$k} = $ENV{$k};
        delete $req_vars{$k};
    } else {
        $req_info |= $req_vars{$k};
    }
}

#
# Set build variables
#

$v{DEB_BUILD_ARCH} = get_raw_build_arch()
    if (action_needs(INFO_BUILD_ARCH_NAME));
($v{DEB_BUILD_ARCH_ABI}, $v{DEB_BUILD_ARCH_LIBC},
 $v{DEB_BUILD_ARCH_OS}, $v{DEB_BUILD_ARCH_CPU}) = debarch_to_debtuple($v{DEB_BUILD_ARCH})
    if (action_needs(INFO_BUILD_ARCH_TUPLE));
($v{DEB_BUILD_ARCH_BITS}, $v{DEB_BUILD_ARCH_ENDIAN}) = debarch_to_abiattrs($v{DEB_BUILD_ARCH})
    if (action_needs(INFO_BUILD_ARCH_ATTR));

$v{DEB_BUILD_MULTIARCH} = debarch_to_multiarch($v{DEB_BUILD_ARCH})
    if (action_needs(INFO_BUILD_MULTIARCH));

if (action_needs(INFO_BUILD_GNU_TUPLE)) {
  $v{DEB_BUILD_GNU_TYPE} = debarch_to_gnutriplet($v{DEB_BUILD_ARCH});
  ($v{DEB_BUILD_GNU_CPU}, $v{DEB_BUILD_GNU_SYSTEM}) = split(/-/, $v{DEB_BUILD_GNU_TYPE}, 2);
}

#
# Set host variables
#

# First perform some sanity checks on the host arguments passed.

($req_host_arch, $req_host_gnu_type) = check_arch_coherency($req_host_arch, $req_host_gnu_type);

# Proceed to compute the host variables if needed.

$v{DEB_HOST_ARCH} = $req_host_arch || get_raw_host_arch()
    if (action_needs(INFO_HOST_ARCH_NAME));
($v{DEB_HOST_ARCH_ABI}, $v{DEB_HOST_ARCH_LIBC},
 $v{DEB_HOST_ARCH_OS}, $v{DEB_HOST_ARCH_CPU}) = debarch_to_debtuple($v{DEB_HOST_ARCH})
    if (action_needs(INFO_HOST_ARCH_TUPLE));
($v{DEB_HOST_ARCH_BITS}, $v{DEB_HOST_ARCH_ENDIAN}) = debarch_to_abiattrs($v{DEB_HOST_ARCH})
    if (action_needs(INFO_HOST_ARCH_ATTR));

$v{DEB_HOST_MULTIARCH} = debarch_to_multiarch($v{DEB_HOST_ARCH})
    if (action_needs(INFO_HOST_MULTIARCH));

if (action_needs(INFO_HOST_GNU_TUPLE)) {
    if ($req_host_gnu_type eq '') {
        $v{DEB_HOST_GNU_TYPE} = debarch_to_gnutriplet($v{DEB_HOST_ARCH});
    } else {
        $v{DEB_HOST_GNU_TYPE} = $req_host_gnu_type;
    }
    ($v{DEB_HOST_GNU_CPU}, $v{DEB_HOST_GNU_SYSTEM}) = split(/-/, $v{DEB_HOST_GNU_TYPE}, 2);

    my $host_gnu_type = get_host_gnu_type();

    warning(g_('specified GNU system type %s does not match CC system ' .
               'type %s, try setting a correct CC environment variable'),
            $v{DEB_HOST_GNU_TYPE}, $host_gnu_type)
        if ($host_gnu_type ne '') && ($host_gnu_type ne $v{DEB_HOST_GNU_TYPE});
}

#
# Set target variables
#

# First perform some sanity checks on the target arguments passed.

($req_target_arch, $req_target_gnu_type) = check_arch_coherency($req_target_arch, $req_target_gnu_type);

# Proceed to compute the target variables if needed.

$v{DEB_TARGET_ARCH} = $req_target_arch || $v{DEB_HOST_ARCH} || $req_host_arch || get_raw_host_arch()
    if (action_needs(INFO_TARGET_ARCH_NAME));
($v{DEB_TARGET_ARCH_ABI}, $v{DEB_TARGET_ARCH_LIBC},
 $v{DEB_TARGET_ARCH_OS}, $v{DEB_TARGET_ARCH_CPU}) = debarch_to_debtuple($v{DEB_TARGET_ARCH})
    if (action_needs(INFO_TARGET_ARCH_TUPLE));
($v{DEB_TARGET_ARCH_BITS}, $v{DEB_TARGET_ARCH_ENDIAN}) = debarch_to_abiattrs($v{DEB_TARGET_ARCH})
    if (action_needs(INFO_TARGET_ARCH_ATTR));

$v{DEB_TARGET_MULTIARCH} = debarch_to_multiarch($v{DEB_TARGET_ARCH})
    if (action_needs(INFO_TARGET_MULTIARCH));

if (action_needs(INFO_TARGET_GNU_TUPLE)) {
    if ($req_target_gnu_type eq '') {
        $v{DEB_TARGET_GNU_TYPE} = debarch_to_gnutriplet($v{DEB_TARGET_ARCH});
    } else {
        $v{DEB_TARGET_GNU_TYPE} = $req_target_gnu_type;
    }
    ($v{DEB_TARGET_GNU_CPU}, $v{DEB_TARGET_GNU_SYSTEM}) = split(/-/, $v{DEB_TARGET_GNU_TYPE}, 2);
}


if ($action eq 'list') {
    foreach my $k (sort keys %arch_vars) {
	print "$k=$v{$k}\n";
    }
} elsif ($action eq 'print-set') {
    if ($print_format eq 'shell') {
        foreach my $k (sort keys %arch_vars) {
            print "$k=$v{$k}; ";
        }
        print 'export ' . join(' ', sort keys %arch_vars) . "\n";
    } elsif ($print_format eq 'make') {
        foreach my $k (sort keys %arch_vars) {
            print "export $k = $v{$k}\n";
        }
    }
} elsif ($action eq 'print-unset') {
    if ($print_format eq 'shell') {
        print 'unset ' . join(' ', sort keys %arch_vars) . "\n";
    } elsif ($print_format eq 'make') {
        foreach my $k (sort keys %arch_vars) {
            print "undefine $k\n";
        }
    }
} elsif ($action eq 'equal') {
    exit !debarch_eq($v{DEB_HOST_ARCH}, $req_eq_arch);
} elsif ($action eq 'is') {
    exit !debarch_is($v{DEB_HOST_ARCH}, $req_is_arch);
} elsif ($action eq 'command') {
    @ENV{keys %v} = values %v;
    ## no critic (TestingAndDebugging::ProhibitNoWarnings)
    no warnings qw(exec);
    exec @ARGV or syserr(g_('unable to execute %s'), "@ARGV");
} elsif ($action eq 'query') {
    print "$v{$req_variable_to_print}\n";
} elsif ($action eq 'list-known') {
    foreach my $arch (get_valid_arches()) {
        my ($bits, $endian) = debarch_to_abiattrs($arch);

        next if $req_match_endian and $endian ne $req_match_endian;
        next if $req_match_bits and $bits ne $req_match_bits;
        next if $req_match_wildcard and not debarch_is($arch, $req_match_wildcard);

        print "$arch\n";
    }
}
