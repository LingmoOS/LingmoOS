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
iarch=$(dpkg --print-architecture)

cd "$vardir/methods/$method"

. ./shvar.$option

#debug() { echo "DEBUG: $@"; }
debug() {
  true
}

packages=0
for f in main ctb nf lcl; do
  eval 'this_packages=$p_'$f'_packages'

  if [ -n "$this_packages" ]; then
    packages=1
  fi
done

if [ $packages eq 0 ]; then
  echo '
No Packages files available, cannot update available packages list.
Hit RETURN to continue.  '
  read response
  exit 0
fi

xit=1
trap '
  for area in main ctb nf lcl; do
    rm -f packages-$area
  done
  if [ -n "$umount" ]; then
    umount "$umount" >/dev/null 2>&1
  fi
  exit $xit
' 0

if [ ! -b "$p_blockdev" ]; then
  loop=",loop"
fi

if [ -n "$p_blockdev" ]; then
  umount="$p_mountpoint"
  mount -rt "$p_fstype" -o nosuid,nodev${loop} "$p_blockdev" "$p_mountpoint"
fi

updatetype=update

if [ -z "$p_multi" ]; then
  exit 1
fi

for f in main ctb nf lcl; do
  eval 'this_packages=$p_'$f'_packages'
  case "$this_packages" in
  '')
    continue
    ;;
  scan)
    eval 'this_binary=$p_'$f'_binary'
    if [ -z "$this_binary" ]; then
      continue
    fi
    if [ "$updatetype" = update ]; then
      dpkg --clear-avail
      updatetype=merge
    fi
    echo Running dpkg --record-avail -R "$p_mountpoint$this_binary"
    dpkg --record-avail -R "$p_mountpoint$this_binary"
    ;;
  *)
    packagesfile="$p_mountpoint$this_packages"
    case "$packagesfile" in
    *.gz)
      echo -n "Uncompressing $packagesfile ... "
      zcat <"$packagesfile" >packages-$f
      echo done.
      dpkg --$updatetype-avail packages-$f
      updatetype=merge
      ;;
    '')
      ;;
    *)
      dpkg --$updatetype-avail "$packagesfile"
      updatetype=merge
      ;;
    esac
  ;;
  esac
done

cp -f $vardir/available $vardir/methods/$method

echo -n 'Update OK.  Hit RETURN.  '
read response

xit=0
