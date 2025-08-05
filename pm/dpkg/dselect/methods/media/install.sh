#!/bin/sh
#
# Copyright © 1995-1998 Ian Jackson <ijackson@chiark.greenend.org.uk>
# Copyright © 1998 Heiko Schlittermann <hs@schlittermann.de>
#
# This is free software; you can redistribute it and/or modify
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

set -e
vardir="$1"
method=$2
option=$3

cd "$vardir/methods/$method"

. ./shvar.$option

#debug() { echo "DEBUG: $@"; }
debug() {
  true
}
iarch=$(dpkg --print-architecture)

# 1/ mountpoint
# 2/ hierarchy
getdisklabel () {
  debug "$1" "$2"
  if [ -f $1/.disk/info ]; then
   echo -n $(head -1 "$1/.disk/info")
  elif [ -f $1$2/.disk/info ]; then
    echo -n $(head -1 "$1$2/.disk/info")
  else
    echo -n 'Non-Debian disc'
  fi
}

xit=1

do_umount() {
  if [ -n "$umount" ]; then
    echo umount "$umount"
    #">/dev/null" "2>&1"
  fi
}

do_mount() {
  if [ ! -b "$p_blockdev" ]; then
    loop=",loop"
  fi

  if [ -n "$p_blockdev" ]; then
    umount="$p_mountpoint"
    echo mount -rt iso9660 -o nosuid,nodev${loop} "$p_blockdev" "$p_mountpoint"\; umount="$p_mountpoint"
  fi
}

trap 'eval $(do_umount); exit $xit' 0

eval $(do_mount)

predep="$vardir/predep-package"
while true; do
  thisdisk="$(getdisklabel ${p_mountpoint} ${p_hierbase})"
  set +e
  dpkg --predep-package >"$predep"
  rc=$?
  set -e
  if test $rc = 1; then
    break
  fi
  test $rc = 0

  perl -e '
	($binaryprefix,$predep,$thisdisk) = @ARGV;
	open(P, "< $predep") or die "cannot open $predep: $!\n";
	while (<P>) {
		s/\s*\n$//;
		$package = $_ if s/^Package: //i;
		/^X-Medium:\s+(.*)\s*/ and $medium = $1;
		@filename = split(/ /,$_) if s/^Filename: //i;
		@msdosfilename = split(/ /,$_) if s/^MSDOS-Filename: //i;
	}
	die "internal error - no package" if length($package) == 0;
	die "internal error - no filename" if not @filename;
	die "internal error - mismatch >@filename< >@msdosfilename<"
		if @filename && @msdosfilename &&
		   @filename != @msdosfilename;
	if ($medium && ($medium ne $thisdisk)) {
		print "

This is
    $thisdisk
However, $package is expected on disc:
    $medium
Please change the discs and press <RETURN>.

";
		exit(1);
	}
	@invoke = (); $| =1;
	for ($i = 0; $i <= $#filename; $i++) {
		$ppart = $i+1;
		print "Looking for part $ppart of $package ... ";
		if (-f "$binaryprefix$filename[$i]") {
			$print = $filename[$i];
			$invoke = "$binaryprefix$filename[$i]";
		} elsif (-f "$binaryprefix$msdosfilename[$i]") {
			$print = $msdosfilename[$i];
			$invoke = "$binaryprefix$msdosfilename[$i]";
		} else {
			$base = $filename[$i]; $base =~ s,.*/,,;
			$msdosbase = $msdosfilename[$i]; $msdosbase =~ s,.*/,,;
			$c = open(X, "-|"));
			if (not defined $c) {
				die "failed to fork for find: $!\n";
			}
			if (!$c) {
				exec("find", "-L",
				     length($binaryprefix) ? $binaryprefix : ".",
				     "-name",$base,"-o","-name",$msdosbase);
				die "failed to exec find: $!\n";
			}
			while (chop($invoke = <X>)) { last if -f $invoke; }
			$print = $invoke;
			if (substr($print,0,length($binaryprefix)+1) eq
			    "$binaryprefix/") {
				$print = substr($print,length($binaryprefix));
			}
		}
		if (!length($invoke)) {
			warn "

Cannot find the appropriate file(s) anywhere needed to install or upgrade
package $package. Expecting version $version or later, as listed in the
Packages file.

Perhaps the package was downloaded with an unexpected name? In any case,
you must find the file(s) and then either place it with the correct
filename(s) (as listed in the Packages file or in $vardir/available)
and rerun the installation, or upgrade the package by using
\"dpkg --install --auto-deconfigure" by hand.

";
			exit(1);
		}
		print "$print\n";
		push(@invoke,$invoke);
	}
	print "Running dpkg -iB for $package ...\n";
	exec("dpkg","-iB","--",@invoke);
	die "failed to exec dpkg: $!\n";
  ' -- "$p_mountpoint$p_hierbase" "$predep" "$thisdisk"
done

perl -e '
	$SIG{INT} = sub { cd $vardir; unlink <tmp/*>; exit 1; };
	$| = 1;
	my ($vardir, $mountpoint, $hierbase, $mount, $umount) = @ARGV;
	my $line;
	my $AVAIL = "$vardir/methods/media/available";
	my $STATUS = "$vardir/status";
	my %Installed, %Filename, %Medium;
	print "Get currently installed package versions...";
	open(IN, "$STATUS") or die "cannot open $STATUS: $!\n";
	$line = 0;
	{ local $/ = "";
	while (<IN>) {
		my %status;
		my @pstat;
		$line++ % 20 or print ".";
		s/\n\s+/ /g;
		%status =  ("", split /^(\S*?):\s*/m, $_);
		map { chomp $status{$_}; $status{$_} =~ s/^\s*(.*?)\s*$/$1/;} keys %status;
		@pstat = split(/ /, $status{Status});
		next unless ($pstat[0] eq "install");
		if ($pstat[2] eq "config-files" || $pstat[2] eq "not-installed") {
		    $Installed{$status{Package}} = "0.0";
		} else {
		    $Installed{$status{Package}} = $status{Version} || "" ;
		}
	}; }
	print "\nGot ", scalar keys %Installed, " installed/pending packages\n";
	print "Scanning available packages...";
	$line = 0;
	open(IN, "$AVAIL") or die("Cannot open $AVAIL: $!\n");
	{ local $/ = "";
	while (<IN>) {
		my $updated;
		 $line++ % 20 or print ".";

		 s/\n\s+/ /g;
		 %avail   =  ("", split /^(\S*?):\s*/m, $_);
		 map { chomp $avail{$_}; $avail{$_} =~ s/^\s*(.*?)\s*$/$1/;} keys %avail;

		 next unless defined $Installed{$avail{Package}};

		 system "dpkg", "--compare-versions", $avail{Version}, "gt", $Installed{$avail{Package}};
		 $updated = ($? == 0);
		 #print "$avail{Package}(" . ($updated ? "+" : "=") . ") ";
		 $updated or next;

		 $Filename{$avail{Package}} = $avail{Filename};

		 next unless defined $avail{"X-Medium"};
		 ${Medium{$avail{"X-Medium"}}} or ${Medium{$avail{"X-Medium"}}} = [];
		 push @{${Medium{$avail{"X-Medium"}}}}, $avail{Package};
	}; };
	print "\n";

	if (@_ = keys(%Medium)) {
		    print "You will need the following distribution disc(s):\n",
			join (", ", @_), "\n";
	}

	foreach $need (sort @_) {
		if (-r "$mountpoint/.disk/info") {
			open(IN, "$mountpoint/.disk/info") or die("$mountpoint/.disk/info: $!\n");
		} else {
			open(IN, "$mountpoint/$hierbase/.disk/info") or die("$mountpoint/$hierbase/.disk/info: $!\n");
		}
		$disk = <IN>; chomp $disk; close(IN);

		print "Processing disc\n   $need\n";

		while ($disk ne $need) {
			print "Wrong disc.  This is disc\n    $disk\n";
			print "However, the needed disc is\n    $need\n";
			print "Please change the discs and press <RETURN>\n";
			system($umount);
			<STDIN>;
			system($mount);
			$? and warn("cannot mount $mount\n");
		} continue {
			if (-r "$mountpoint/.disk/info") {
			    open(IN, "$mountpoint/.disk/info") or die("$mountpoint/.disk/info: $!\n");
			} else {
			    open(IN, "$mountpoint/$hierbase/.disk/info") or die("$mountpoint/$hierbase/.disk/info: $!\n");
			}
			$disk = <IN>; chomp $disk; close(IN);
		}

		-d "tmp" || mkdir "tmp", 0755 or die("Cannot mkdir tmp: $!\n");
		unlink <tmp/*>;

		print "creating symlinks...\n";
		foreach (@{$Medium{$need}}) {
			($basename = $Filename{$_}) =~ s/.*\///;
			symlink "$mountpoint/$hierbase/$Filename{$_}",
				"tmp/$basename";
		}
		chdir "tmp" or die "cannot chdir to tmp: $!\n";
		system "dpkg", "-iGROEB", ".";
		unlink <*>;
		chdir "..";

		if ($?) {
			print "\nThe dpkg run produced errors. Please state whether to\n",
			      "continue with the next media disc. [Y/n]: ";
			$answer = <STDIN>;
			exit 1 if $answer =~ /^n/i;
			$ouch = $?;
		}
	}

	exit $ouch;

' "$vardir" "$p_mountpoint" "$p_hierbase" "$(do_mount)" "$(do_umount)"

echo -n 'Installation OK.  Hit RETURN.  '
read response

xit=0
