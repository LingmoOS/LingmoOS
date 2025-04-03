package KernelWedge;

use strict;
use warnings;

BEGIN {
	use Exporter ();
	our @ISA = qw(Exporter);
	our @EXPORT_OK = qw(CONTROL_FIELDS CONFIG_DIR DEFCONFIG_DIR
			    read_package_lists read_kernel_versions
			    for_each_package);
}

use constant CONTROL_FIELDS => qw(
	Package Package-Type Provides Depends Architecture Kernel-Version
	Section Priority Description
);

use constant DEFCONFIG_DIR => $ENV{KW_DEFCONFIG_DIR};
if (!defined(DEFCONFIG_DIR)) {
	die "Required environment variable \$KW_DEFCONFIG_DIR is not defined";
}
use constant CONFIG_DIR => ($ENV{KW_CONFIG_DIR} || '.');

sub read_package_list
{
	my ($packages, $order, $file) = @_;

	sub merge_package
	{
		my ($packages, $order, $pkg) = @_;
		if (not exists $packages->{$pkg->{Package}}) {
			$packages->{$pkg->{Package}} = {};
			push @$order, $pkg->{Package};
		}
		my $real_pkg = $packages->{$pkg->{Package}};
		foreach (keys(%$pkg)) {
			$real_pkg->{$_} = $pkg->{$_};
		}
	}

	open(LIST, $file) || die "package-list: $!";
	my $field;
	my %pkg;
	while (<LIST>) {
		chomp;
		next if /^#/;

		if (/^(\S+):\s*(.*)/) {
			$field=$1;
			my $val=$2;
			if (! grep { $field =~ /^\Q$_\E(_.+)?$/ } CONTROL_FIELDS) {
				die "unknown field, $field";
			}
			$pkg{$field}=$val;
		}
		elsif (/^$/) {
			if (%pkg) {
				merge_package($packages, $order, \%pkg);
				%pkg=();
			}
		}
		elsif (/^(\s+.*)/) {
			# continued field
			$pkg{$field}.="\n".$1;
		}
	}
	if (%pkg) {
		merge_package($packages, $order, \%pkg);
	}
	close LIST;
}

sub read_package_lists {
	my %packages;
	my @order;

	read_package_list(\%packages, \@order, DEFCONFIG_DIR . "/package-list")
		unless DEFCONFIG_DIR eq CONFIG_DIR;
	read_package_list(\%packages, \@order, CONFIG_DIR . "/package-list");

	return [map {$packages{$_}} @order];
}

sub read_kernel_versions {
	my ($fixkernelversion) = @_;
	my @versions;

	open(KVERS, CONFIG_DIR . "/kernel-versions") || die "kernel-versions: $!";
	while (<KVERS>) {
		chomp;
		next if /^#/ || ! length;

		my @fields = split(' ', $_, 6);
		my ($arch, $kernelversion, $flavour) = @fields;
		if (! length $arch || ! length $kernelversion || ! length $flavour) {
			die "parse error";
		}
		push @versions, \@fields;
	}
	close KVERS;

	return \@versions;
}

sub for_each_package {
	my ($packages, $versions, $fn) = @_;

	foreach my $ver (@$versions) {
		my ($arch, $kernelversion, $flavour) = @$ver;
		foreach my $pkg (@$packages) {
			# Used to get a field of the package, looking first for
			# architecture-specific fields.
			my $package = sub {
				my $field=shift;
				return $pkg->{$field."_".$flavour}
				if exists $pkg->{$field."_".$flavour};
				return $pkg->{$field."_".$arch."_".$flavour}
				if exists $pkg->{$field."_".$arch."_".$flavour};
				return $pkg->{$field."_".$arch}
				if exists $pkg->{$field."_".$arch};
				return $pkg->{$field}
				if exists $pkg->{$field};
				return undef;
			};

			# Check for a modules list file for this architecture and
			# package.
			my $modlistdir="";
			if (-d (CONFIG_DIR . "/modules/$arch-$flavour")) {
				$modlistdir = CONFIG_DIR . "/modules/$arch-$flavour";
			}
			elsif (-d (CONFIG_DIR . "/modules/$flavour")) {
				$modlistdir = CONFIG_DIR . "/modules/$flavour";
			}
			else {
				$modlistdir = CONFIG_DIR . "/modules/$arch";
			}

			next unless -e "$modlistdir/".$package->("Package");

			$fn->($arch, $kernelversion, $flavour, $modlistdir,
			      $package);
		}
	}
}

1;
