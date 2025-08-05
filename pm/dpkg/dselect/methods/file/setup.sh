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
tp="$(mktemp --tmpdir $method.XXXXXXXXXX)"

iarch=$(dpkg --admindir $vardir --print-architecture)

xit=1
trap '
  rm -f $tp.?
  exit $xit
' 0

if ls -d "$tp.?" >/dev/null 2>&1; then
  rm $tp.?
fi

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

outputparam () {
  echo "$2" | sed -e "s/'/'\\\\''/; s/^/$1='/; s/$/'/" >&3
}

intrkey="$(stty -a | sed -n 's/.*intr = \([^;]*\);.*/\1/p')"
echo "
If you make a mistake, use the interrupt key ($intrkey) to abort.
"

# State variables, “best first”
#  {main,ctb,nf,lcl}_{packages,binary}
#   Empty before we've found them or if they're not available,
#   set to the relevant bit under mountpoint otherwise.
#  hierbase
#   A directory containing a Debian FTP site mirror tree for ONE distribution.
#	eg /pub/debian/dists/stable
#  mountpoint
#   The mountpoint for the filesystem containing the stuff
#   empty or unset if we don't know yet, or if we haven't mounted anything;
#   may also be empty if ‘directory’ was set.
#  fstype
#   The filesystem type to use.

if [ -f shvar.$option ]; then
  . ./shvar.$option
fi

if [ -n "$mountpoint" ]; then
  # We must have $mountpoint
  echo \
"All directory names should be entered relative to the root of the
$fstype filesystem.
"
fi

while true; do
  echo \
"In order to make it easy to find the relevant files, it is preferred
to install from a straightforward copy of the Debian distribution.
To use this, it is required to know where the top level of that copy of
the distribution is (eg. 'debian/dists/stable') - this directory usually
contains the Packages-Master file.

If you do not have a straightforward copy of the distribution available
just answer 'none' and each needed part will be prompted individually."
  defhierbase=none
  # maybe ask for debian/dists and then show and ask for available dists
  # eg. {stable,frozen,unstable,bo,hamm,slink}
  if [ -n "$p_hierbase" ]; then
    if [ -d "$mountpoint/$p_hierbase/main/binary-$iarch" ]; then
      echo "
Last time you said '$p_hierbase', and that looks plausible."
      defhierbase="$p_hierbase"
    else
      echo "
Last time you said '$p_hierbase', but that doesn't look plausible,
since '$p_hierbase/main/binary-$iarch' doesn't seem to exist."
    fi
  fi
  if [ none = "$defhierbase" ]; then
    if [ -d "$mountpoint/debian/dists/stable/main/binary-$iarch" ]; then
      echo "
'/debian/dists/stable' exists and looks plausible, so that's the default."
      defhierbase=/debian/dists/stable
    elif [ -d "$mountpoint/dists/stable/main/binary-$iarch" ]; then
      echo "
'/dists/stable' exists and looks plausible, so that's the default."
      defhierbase=/dists/stable
    fi
  fi
  echo -n \
"Distribution top level ? [$defhierbase]  "
  read response
  if [ -z "$response" ]; then
    response="$defhierbase"
  fi
  if [ none = "$response" ]; then
    hierbase=""
    break
  elif [ -d "$mountpoint/$response/main/binary-$iarch" ]; then
    hierbase="$(echo "$response" | sed -e 's:/*$::; s:^/*:/:')"
    break
  fi
  echo \
"$response/main/binary-$iarch does not exist.
"
done

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
  # eg:   main             "$hierbase/main/binary-$iarch"
  # checks whether $2 contains *.deb
  if ! [ -d "$mountpoint$2/" ]; then
    echo "'$2' does not exist."
    return
  fi

  if ! ( find -L "$mountpoint$2/" -name '*.deb' -print \
       | head -n 1 ) 2>/dev/null  | grep . >/dev/null; then
    echo "'$2' does not contain any *.deb packages.  Hmmpf."
    return
  fi
  echo "Using '$2' as $1 binary dir."
  this_binary="$2"
}

find_area () {
  # args: area-in-messages area-in-vars subdirectory-in-hier
  #       last-time-binary last-time-packages
  # eg:   main             main         main
  #       "$p_main_binary" "$p_main_packages"

  this_binary=''
  this_packages=''
  if [ -n "$hierbase" ]; then
    check_binary $1 "$hierbase/$3/binary-$iarch"
  fi

  if [ $2 = lcl ] && [ -z "$this_binary" ]; then
    echo "
Note: By default there is no 'local' directory. It is intended for
packages you made yourself."
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
area (this directory is named '$3/binary-$iarch' on the distribution site) ?
Say 'none' if this area is not available."
    if [ $2 != main ] && [ -z "$defaultbinary" ]; then
      defaultbinary=none
    fi
    echo -n \
"Enter _$1_ binary dir. [$4]
 ?  "
    read response
    if [ -z "$response" ] && [ -n "$defaultbinary" ]; then
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
    for f in Packages.gz packages.gz Packages packages; do
      if [ -f "$mountpoint/$this_binary/$f" ]; then
        echo "Using '$this_binary/$f' for $1."
        this_packages="$this_binary/$f"
        break
      fi
    done
    while [ -z "$this_packages" ]; do
      echo -n "
Cannot find the $1 'Packages' file. The information in the
'Packages' file is important for package selection during new
installations, and is very useful for upgrades.

If you overlooked it when downloading you should do get it now and
return to this installation procedure when you have done so: you will
find one Packages file and one Packages.gz file -- either will do --
in the 'binary-$iarch' subdirectory of each area on the FTP sites and
media discs. Alternatively (and this will be rather slow) the packages in
the distribution area can be scanned - say 'scan' if you want to do so.

You need a separate Packages file from each of the distribution areas
you wish to install.

Where is the _$1_ 'Packages' file (if none is available, say 'none')
[$5]
 ?  "
      read response
      if [ -z "$response" ] && [ -n "$5" ]; then
        response="$5"
      fi
      case "$response" in
      '')
        continue
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
}

find_area main main main "$p_main_binary" "$p_main_packages"
find_area contrib ctb contrib "$p_ctb_binary" "$p_ctb_packages"
find_area non-free-firmware nff non-free-firmware "$p_nff_binary" "$p_nff_packages"
find_area non-free nf non-free "$p_nf_binary" "$p_nf_packages"
find_area local lcl local "$p_lcl_binary" "$p_lcl_packages"

echo -n '
Hit RETURN to continue.  '
read response

exec 3>shvar.$option.new

outputparam p_fstype "$fstype"
outputparam p_mountpoint "$mountpoint"
outputparam p_hierbase "$hierbase"

outputparam p_main_packages "$main_packages"
outputparam p_main_binary "$main_binary"
outputparam p_ctb_packages "$ctb_packages"
outputparam p_ctb_binary "$ctb_binary"
outputparam p_nf_packages "$nf_packages"
outputparam p_nf_binary "$nf_binary"
outputparam p_nff_packages "$nff_packages"
outputparam p_nff_binary "$nff_binary"
outputparam p_lcl_packages "$lcl_packages"
outputparam p_lcl_binary "$lcl_binary"

mv shvar.$option.new shvar.$option

xit=0
