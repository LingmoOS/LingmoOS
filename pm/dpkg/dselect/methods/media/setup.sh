#!/bin/sh
#
# Copyright © 1995-1998 Ian Jackson <ijackson@chiark.greenend.org.uk>
# Copyright © 1998 Heiko Schlittermann <hs@schlittermann.de>
# Copyright © 1998-1999 Martin Schulze <joey@infodrom.north.de>
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

test -d "$vardir/methods/$method" || mkdir "$vardir/methods/$method"
cd "$vardir/methods/$method"
tp="$(mktemp --tmpdir $method.XXXXXXXXXX)"

iarch=$(dpkg --print-architecture)
dist=stable

xit=1
trap '
  rm -f $tp.?
  if [ -n "$umount" ]; then
    umount "$umount" >/dev/null 2>&1
  fi
  exit $xit
' 0

if ls -d "$tp.?" >/dev/null 2>&1; then
  rm $tp.?
fi

#debug() { echo "DEBUG: $@"; }
debug() {
  true
}
ismulti() {
  debug $1 $2; test -e "$1/.disk/info" || test -e "$1$2/.disk/info"
}

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

yesno () {
  while true; do
    echo -n "$2 [$1]  "
    read response
    if [ -z "$response" ]; then
      response="$1"
    fi
    case "$response" in
    [Nn]*)
      yesno=no
      return
      ;;
    [Yy]*)
      yesno=yes
      return
      ;;
    esac
  done
}

getblockdev () {
  mountpoint="$vardir/methods/mnt"
  if [ -z "$defaultdevice" ]; then
    defaultdevice="$newdefaultdevice"
  elif [ "$defaultdevice" != "$newdefaultdevice" ]; then
    echo "Last time you specified installation from $defaultdevice."
  fi
  promptstring="$1"
  while [ -z "$blockdevice" ]; do
    echo -n "$promptstring [$defaultdevice]:  "
    read response
    if [ -z "$response" ]; then
      response="$defaultdevice"
    fi
    if [ ! -b "$response" ]; then
      echo "$response is not a block device - will try as loopback."
      loop=",loop"
    fi
    tryblockdevice="$response"
    fstype=iso9660
    umount="$mountpoint"
    if mount -rt "$fstype" -o nosuid,nodev$loop "$tryblockdevice" "$mountpoint"
    then
      echo
      blockdevice="$tryblockdevice"
    else
      umount=""
      echo "Unable to mount $tryblockdevice on $mountpoint, type $fstype."
    fi
  done
}

outputparam () {
  echo "$2" | sed -e "s/'/'\\\\''/; s/^/$1='/; s/$/'/" >&3
}

## MAIN
intrkey="$(stty -a | sed -n 's/.*intr = \([^;]*\);.*/\1/p')"
echo "
If you make a mistake, use the interrupt key ($intrkey) to abort.
"

# State variables, “best first”
#  {main,ctb,nf,lcl}_{packages,binary}
#   Empty before we've found them or if they're not available,
#   set to the relevant bit under mountpoint otherwise.
#  hierbase
#   A directory containing a Debian FTP site mirror tree.
#  mountpoint
#   The mountpoint for the filesystem containing the stuff
#   empty or unset if we don't know yet, or if we haven't mounted anything;
#   may also be empty if ‘directory’ was set.
#  blockdevice
#   The actual block device to mount.
#  fstype
#   The filesystem type to use.
#  defaultdevice
#   The default block device to mount.

p_usedevel=no
if [ -f shvar.$option ]; then
  . ./shvar.$option
  defaultdevice="$p_blockdev"
  usedevel="$p_usedevel"
fi

mount >$tp.m
sed -n 's/ ([^)]*)$//; s/^[^ ]* on //; s/ type iso9660$//p' <$tp.m >$tp.l
ncdroms=$(wc -l <$tp.l)
if [ $ncdroms -gt 1 ]; then
  response=""
  while [ -z "$response" ]; do
    echo 'Several media discs (ISO9660 filesystems) are mounted:'
    grep -E 'type iso9660 \([^)]*\)$' <$tp.m | nl
    echo -n "Is it any of these ?  Type a number, or 'n' for none.  "
    read response
    response="$(echo "$response" | sed -e 's/[ 	]*$//')"
    if expr "$response" : '[0-9][0-9]*$' >/dev/null && \
       [ $response -ge 1 -a $response -le $ncdroms ]; then
             mountpoint="$(sed -n $response'p' <$tp.l)"
      echo
    elif expr "$response" : '[Nn]' >/dev/null; then
      mountpoint=""
    else
      response=""
    fi
  done
elif [ $ncdroms = 1 ]; then
  mountpoint="$(cat $tp.l)"
  perl -ne 'print if s/ type iso9660 \([^)]*\)$// && s/ on .*$//;' \
    <$tp.m >$tp.d
  blockdevice="$(cat $tp.d)"
  yesno yes \
    "Found a media disc: $blockdevice, mounted on $mountpoint. Is it the right one?"
  if [ $yesno = no ]; then
    echo 'Unmounting it ...'
    umount="$mountpoint"
    while true; do
      echo -n 'Please insert the right disc, and hit return:  '
      read response
      if mount -rt iso9660 -o nosuid,nodev \
         "$blockdevice" "$mountpoint"; then
        echo
        break
      fi
    done
  fi
fi
if [ -z "$mountpoint" ]; then
  if [ -b /dev/cdrom ]; then
    echo 'Found that /dev/cdrom exists and is a block device.'
    newdefaultdevice=/dev/cdrom
  fi
  getblockdev 'Insert the media and enter the block device name'
fi

if [ -n "$mountpoint" ]; then
  # We must have $mountpoint
  echo \
'All directory names should be entered relative to the root of the media disc.
'
fi

# now try to get the users idea where the debian
# hierarchy start below the mointpoint

debug "mountpoint: $mountpoint"
while true; do
  if ismulti "${mountpoint}" "${hierbase}"; then
    multi=yes
  fi

  echo \
"Need to know where on the media disc the top level of the Debian
distribution is - this will usually contain the 'dists' directory.

If the media disc is badly organized and doesn't have a straightforward copy of
the distribution you may answer 'none' and the needed parts will be prompted
individually."

  defhierbase=none
  if [ -n "$p_hierbase" ]; then
    if [ -d "$mountpoint/$p_hierbase/dists/$dist/main/binary-$iarch" \
         -o -n "$multi" ]; then
      echo "Last time you said '$p_hierbase', and that looks plausible."
      defhierbase="$p_hierbase"
    else
      echo "
Last time you said '$p_hierbase', but that doesn't look plausible,
since '$p_hierbase/dists/$dist/main/binary-$iarch' doesn't seem to exist.
And it does not appear that you are using a multiple media set."
    fi
  fi

  # at this point defhierbase is set if it looks plausible
  # if ‘none’ was entered, we assume a media with a debian/ directory

  if [ none = "$defhierbase" -a -d "$mountpoint/debian/dists/$dist/main/binary-$iarch" ]
  then
    echo "'/debian' exists and looks plausible, so that's the default."
    defhierbase=/debian
  fi

  echo -n "Distribution top level ? [$defhierbase]  "
  read response
  if [ -z "$response" ]; then
    response="$defhierbase"
  fi
  if [ none = "$response" ]; then
    hierbase=""
    break
  elif ismulti "$mountpoint" "$response" && [ -z "$multi" ]; then
    multi=yes
  fi

  if ! [ -d "$mountpoint/$response/dists/$dist/main/binary-$iarch" \
         -o -n "$multi" ]; then
    echo \
"Neither $response/dists/$dist/main/binary-$iarch does not exist,
nor are you using a multiple media set"
    break
  fi

  hierbase="$(echo "$response" | sed -e 's:/$::; s:^/*:/:; s:/\+:/:g;')"
  debug "hierbase: [$hierbase]"

  if [ -n "$multi" ]; then
    disklabel=$(getdisklabel "$mountpoint" "/$response")
    echo "Ok, this is disc"
    echo "    $disklabel"
    #echo "Updating multiple media contents file cache ..."
    #multi_contentsfile="${mountpoint}/${response}/.disk/contents.gz"
    #zcat "$multi_contentsfile" > disk-contents.$option
  fi

  break;
done

distribution=$dist
if [ -n "$hierbase" ]; then
  if [ -d "$mountpoint/$hierbase/dists/unstable/binary-$iarch" ]; then
    echo \
'
Both a stable released distribution and a work-in-progress
development tree are available for installation.  Would you like to
use the unreleased development tree (this is only recommended for
experts who like to live dangerously and want to help with testing) ?'
    yesno "$p_usedevel" 'Use unreleased development distribution ?'
    usedevel="$yesno"
    if [ "$usedevel" = yes ]; then
      distribution=development
    fi
  else
    usedevel=no
  fi
  echo
fi

case "$hierbase" in
/* )
  ;;
'' )
  ;;
* )
  hierbase="/$hierbase"
  ;;
esac

check_binary () {
  # args: area-in-messages directory
  debug "check_binary($@)"

  if [ ! -d "${mountpoint}$2" -a -z "$multi" ]; then
    echo "'$2' does not exist."
    return
  fi

# In this special case it is ok for a sub-distribution to not contain any
# .deb files. Each media should contain all Packages.cd files but does not
# need to contain the .deb files.
#
#   if ! { find -L "$mountpoint$2" -name '*.deb' -print \
#     | head -1 | grep . ; } >/dev/null 2>&1 && [ -z "$multi" ];
#   then
#     echo "'$2' does not contain any *.deb packages."
#     return
#   fi

  this_binary="$2"
  echo -n "Using '$this_binary' as $1 binary directory"

  if [ -n "$multi" ]; then
    this_disk=$(getdisklabel ${mountpoint} "/$hierbase")
    echo " from disc"
    echo "    '$this_disk'"
  else
    echo ""
  fi
}

find_area () {
  # args: area-in-messages area-in-vars subdirectory-in-hier
  #       last-time-binary last-time-packages
  debug "find_area($@)"
  this_binary=''
  this_packages=''
  this_disk=''
  if [ -n "$hierbase" ]; then
    check_binary $1 $(echo "$hierbase/dists/$3/$1/binary-$iarch" | sed 's:/\+:/:g')
    debug "THIS_BINARY $this_binary"
  fi
  if [ $2 = nf -a -z "$this_binary" ]; then
    echo "
Note: most media distributions of Debian do not include programs available
in the 'non-free' directory of the distribution site.
This is because these programs are under licenses that do not allow source
modification or prevent distribution for profit on a media, or other
restrictions that make them not free software.
If you wish to install these programs you will have to get them from an
alternative source."
  fi
  while [ -z "$this_binary" ]; do
    defaultbinary="$4"
    echo "
Which directory contains the *.deb packages from the $1 distribution
area (this directory is named '$3/binary' on the distribution site) ?
Say 'none' if this area is not available."
    if [ $2 != main -a -z "$defaultbinary" ]; then
      defaultbinary=none
    fi
    echo -n \
"Enter _$1_ binary directory. [$4]
 ?  "
    read response
    if [ -z "$response" -a -n "$defaultbinary" ]; then
      response="$defaultbinary"
    fi
    if [ none = "$response" ]; then
      break
    fi
    case "$response" in
    '' | none)
      continue
      ;;
    esac
    check_binary $1 "$(echo "$response" | sed -e 's:/$::; s:^/*:/:')"
  done
  if [ -n "$this_binary" ]; then
    if [ "$multi" = "yes" ]; then
      for f in Packages.cd.gz packages.cd.gz Packages.cd packages.cd; do
        if [ -f "$mountpoint/$this_binary/$f" ]; then
          this_packages="$this_binary/$f"
          echo "Using '$this_packages' for $1."
          break
        fi
      done
    elif [ -f "${mountpoint}${hierbase}/.disk/packages/$1/Packages.gz" ]; then
      this_packages=$(echo "${hierbase}/.disk/packages/$1/Packages.gz"|sed 's:/\+:/:g')
      echo "Using '${this_packages}' for $1."
    fi
    while [ -z "$this_packages" ]; do
      echo -n "
Cannot find the $1 'Packages.cd' file. The information in the
'Packages.cd' file is important for package selection during new
installations, and is very useful for upgrades.

If you overlooked it when downloading you should do get it now and
return to this installation procedure when you have done so: you will
find one Packages.cd file and one Packages.cd.gz file -- either will do --
in the 'binary' subdirectory of each area on the FTP sites and
media discs. Alternatively (and this will be rather slow) the packages in
the distribution area can be scanned - say 'scan' if you want to do so.

You need a separate Packages.cd file from each of the distribution areas
you wish to install.

Where is the _$1_ 'Packages.cd' file (if none is available, say 'none')
[$5]
 ?  "
      read response
      if [ -z "$response" -a -n "$5" ]; then
        response="$5"
      fi
      case "$response" in
      '')
        break
        ;;
      none)
        break
        ;;
      scan)
        this_packages=scan
        ;;
      /*)
        this_packages="$response"
        ;;
      *)
        this_packages="/$response"
        ;;
      esac
    done
  fi
  eval $2'_binary="$this_binary"'
  eval $2'_packages="$this_packages"'
  eval $2'_disk="$this_disk"'
}

find_area main main "$distribution" "$p_main_binary" "$p_main_packages"
find_area contrib ctb "$distribution" "$p_ctb_binary" "$p_ctb_packages"
find_area non-free-firmware nff "$distribution" "$p_nff_binary" "$p_nff_packages"
find_area non-free nf "$distribution" "$p_nf_binary" "$p_nf_packages"
find_area local lcl local "$p_lcl_binary" "$p_lcl_packages"

echo -n '
Hit RETURN to continue.  '
read response

exec 3>shvar.$option.new

outputparam p_blockdev "$blockdevice"
outputparam p_fstype "$fstype"
outputparam p_mountpoint "$mountpoint"
outputparam p_hierbase "$hierbase"
outputparam p_usedevel "$usedevel"
outputparam p_main_packages "$main_packages"
outputparam p_main_binary "$main_binary"
outputparam p_main_disk "$main_disk"
outputparam p_ctb_packages "$ctb_packages"
outputparam p_ctb_binary "$ctb_binary"
outputparam p_ctb_disk "$ctb_disk"
outputparam p_nff_packages "$nff_packages"
outputparam p_nff_binary "$nff_binary"
outputparam p_nff_disk "$nff_disk"
outputparam p_nf_packages "$nf_packages"
outputparam p_nf_binary "$nf_binary"
outputparam p_nf_disk "$nf_disk"
outputparam p_lcl_packages "$lcl_packages"
outputparam p_lcl_binary "$lcl_binary"
outputparam p_multi "$multi"
outputparam p_multi_contentsfile "$multi_contentsfile"

mv shvar.$option.new shvar.$option

xit=0
