#!/bin/sh
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

set -e
vardir="$1"
method=$2
option=$3

cd "$vardir/methods/file"

. ./shvar.$option

xit=1
trap '
  exit $xit
' 0

predep="$vardir/predep-package"
while true; do
  set +e
  dpkg --admindir "$vardir" --predep-package >"$predep"
  rc=$?
  set -e
  if test $rc = 1; then
    break
  fi
  test $rc = 0

  perl -e '
	($binaryprefix,$predep) = @ARGV;
	$binaryprefix =~ s,/*$,/, if length($binaryprefix);
	open(P, "< $predep") or die "cannot open $predep: $!\n";
	while (<P>) {
		s/\s*\n$//;
		$package = $_ if s/^Package: //i;
		@filename = split(/ /,$_) if s/^Filename: //i;
		@msdosfilename = split(/ /,$_) if s/^MSDOS-Filename: //i;
	}
	die "internal error - no package" if length($package) == 0;
	die "internal error - no filename" if not @filename;
	die "internal error - mismatch >@filename< >@msdosfilename<"
		if @filename && @msdosfilename &&
		   @filename != @msdosfilename;
	@invoke = (); $| = 1;
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
				     length($binaryprefix) ?
				     $binaryprefix : ".",
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
	exec("dpkg","--admindir",$vardir,"-iB","--",@invoke);
	die "failed to exec dpkg: $!\n";
  ' -- "$p_mountpoint$p_main_binary" "$predep"
done

for f in main ctb nf lcl; do
  eval 'this_binary=$p_'$f'_binary'
  if [ -z "$this_binary" ]; then
    continue
  fi
  echo Running dpkg --admindir $vardir -iGROEB "$p_mountpoint$this_binary"
  dpkg --admindir $vardir -iGROEB "$p_mountpoint$this_binary"
done

echo -n 'Installation OK.  Hit RETURN.  '
read response

xit=0
