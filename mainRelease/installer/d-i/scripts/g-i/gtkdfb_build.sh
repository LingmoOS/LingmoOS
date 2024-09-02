#!/bin/bash
# -*-sh-*-
#
#     Copyright (C) 2005 Davide Viti <zinosat@tiscali.it>
#
#     
#     This program is free software; you can redistribute it and/or
#     modify it under the terms of the GNU General Public License
#     as published by the Free Software Foundation; either version 2
#     of the License, or (at your option) any later version.
#     
#     This program is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#     GNU General Public License for more details.
#     
#     You should have received a copy of the GNU General Public License
#     along with this program; if not, write to the Free Software
#     Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
#
#
# Quick and dirty script for rebuilding all the libraries necessary for the
# GTK frontend
#
# First step compilation installs everything inside "./d-i_build"
# second step compilation installs everything inside "./d-i_runtime"

set -e

[ "${PIC}" ] || export PIC=yes
[ "${TOPDIR}" ] || export TOPDIR=$(pwd)/gtkdfb/
export BUILDDIR=${TOPDIR}/d-i_build
export LD_LIBRARY_PATH=${BUILDDIR}/lib
export PKG_CONFIG_PATH=${LD_LIBRARY_PATH}/pkgconfig
LIBRARIES="DirectFB cairo gtk+"

if [ ! -d ${TOPDIR} ] ; then
	mkdir -p ${TOPDIR}
fi

function check() {
   for DIR in ${LIBRARIES} ; do
      if [ ! -d ${TOPDIR}${DIR} ] ; then
         echo "Error: ${DIR} is not installed"
         exit 1
      fi
   done

   if [ ! -d ${TOPDIR}gtk+/gdk/directfb ] ; then
      echo "*** Error: looks like gtk+ has not been patched ***" 
      exit 1      
   fi
}

function pass() {
   step=$1
   HERE=$(pwd)
   for DIR in ${LIBRARIES} ; do
      cd ${TOPDIR}${DIR}

      echo -n "${DIR} (Pass${step})... "

      if [ "${PIC}" = "yes" ]; then
         DEFAULT_OPTS="--with-pic"
      fi

      if [ ${step} = 1 ] ; then
         DEFAULT_OPTS="${DEFAULT_OPTS} --prefix=${BUILDDIR}"
         DESTINATION=""
      else
         DEFAULT_OPTS="${DEFAULT_OPTS} --prefix=/usr --sysconfdir=/etc"
         DESTINATION="DESTDIR=${INSTALLDIR}"
      fi

      case ${DIR} in
         "cairo")
             EXTRA_OPT="--disable-xlib --disable-ps --disable-pdf --enable-directfb=yes"
            ;;
         "DirectFB")
             EXTRA_OPT="--with-inputdrivers=keyboard,linuxinput,ps2mouse,serialmouse"
            ;;
         "gtk+")
            EXTRA_OPT="--with-gdktarget=directfb"
            ;;
         *)
            EXTRA_OPT=""
            ;;
      esac

      sh autogen.sh ${DEFAULT_OPTS} ${EXTRA_OPT} > pass${step}_autogen.log 2>&1
      make > pass${step}_make.log 2>&1
      make install ${DESTINATION} > pass${step}_make_install.log 2>&1

      echo "Done"
      cd ${HERE}
   done
}

case "$1" in
   pass1)
      check;
      export LDFLAGS=-L${BUILDDIR}/lib
      pass 1;
      ;;
   pass2)
      check;
      export INSTALLDIR=${TOPDIR}d-i_runtime
      pass 2;
      ;;
   all)
      check;
      export LDFLAGS=-L${BUILDDIR}/lib
      pass 1;
      export LDFLAGS=""
      export INSTALLDIR=${TOPDIR}d-i_runtime
      pass 2;
      mkdir -p ${INSTALLDIR}/etc/gtk-2.0
      cat ${BUILDDIR}/etc/gtk-2.0/gdk-pixbuf.loaders | sed "s|${BUILDDIR}|/usr|" > ${INSTALLDIR}/etc/gtk-2.0/gdk-pixbuf.loaders
      ;;
   split)
      export INSTALLDIR="${TOPDIR}d-i_split"
      for DIR in ${LIBRARIES} ; do
	  cd ${TOPDIR}${DIR}	  
	  make install DESTDIR="${INSTALLDIR}/${DIR}"
	  cd ..
      done

      # create a tarball for each library 
      for DIR in ${LIBRARIES} ; do
	  cd ${INSTALLDIR}/${DIR}
	  tar czf ${DIR}.tgz *
	  mv ${DIR}.tgz ..
      done
      ;;
   *)
      echo "Usage: $0 {pass1 | pass2 | all | split}"
      echo ""
      echo "pass1          pass1 compilation"
      echo "pass2          pass2 compilation"
      echo "all            pass1 and pass2 compilation"
      echo "split          install each lib in a separate dir"      
      exit 1
esac
