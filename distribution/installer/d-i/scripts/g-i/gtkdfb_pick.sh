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
# Grab all the sources out of CVS and apply the gdk-directfb patch

export TOPDIR=$(pwd)/gtkdfb
export HERE=`pwd`
if [ ! -d ${TOPDIR} ] ; then
	mkdir -p ${TOPDIR}
fi
cd ${TOPDIR}

cvs -d:pserver:anonymous@anoncvs.gnome.org:/cvs/gnome -z3 co -r GTK_2_8_10 gtk+
cvs -d:pserver:anoncvs@cvs.cairographics.org:/cvs/cairo co -D "20060118" cairo
cvs -d:pserver:anonymous@cvs.directfb.org:/cvs/directfb -z3 co -D "20060110" DirectFB
cvs -d:pserver:anonymous@cvs.directfb.org:/cvs/directfb -z3 co -D "20060118" gdk-directfb


# Patch gtk+ as to support gdk-directfb backend
mv gdk-directfb gtk+/gdk/directfb
cd gtk+/gdk/directfb
patch -b -p0 < ./gtk-directfb.patch

cd ${HERE}
