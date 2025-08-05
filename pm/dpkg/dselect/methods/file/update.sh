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

if [ -z "$p_main_packages" ] && [ -z "$p_ctb_packages" ] && \
   [ -z "$p_nf_packages" ] && [ -z "$p_lcl_packages" ]; then
  echo '
No Packages files available, cannot update available packages list.
Hit RETURN to continue.  '
  read response
  exit 0
fi

xit=1
trap '
  rm -f packages-main packages-ctb packages-nf packages-lcl
  exit $xit
' 0

updatetype=update

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
      dpkg --admindir $vardir --clear-avail
      updatetype=merge
    fi
    echo Running dpkg --record-avail -R "$p_mountpoint$this_binary"
    dpkg --admindir $vardir --record-avail -R "$p_mountpoint$this_binary"
    ;;
  *)
    packagesfile="$p_mountpoint$this_packages"
    case "$packagesfile" in
    *.gz)
      echo -n "Uncompressing $packagesfile ... "
      zcat <"$packagesfile" >packages-$f
      echo done.
      dpkg --admindir $vardir --$updatetype-avail packages-$f
      updatetype=merge
      ;;
    '')
      ;;
    *)
      dpkg --admindir $vardir --$updatetype-avail "$packagesfile"
      updatetype=merge
      ;;
    esac
  ;;
  esac
done

echo -n 'Update OK.  Hit RETURN.  '
read response

xit=0
