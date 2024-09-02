#!/bin/bash

ASPELL_URI=ftp://ftp.gnu.org/gnu/aspell/aspell-0.50.5.tar.gz
ASPELL_TGZ=`echo $ASPELL_URI | sed "s|ftp://ftp.gnu.org/gnu/aspell/||"`
wget $ASPELL_URI
tar xzf $ASPELL_TGZ
cd `echo ${ASPELL_TGZ} | sed "s:.tar.gz$::"`
sh configure --enable-static=yes
make install prefix=$TOOLS_PATH
cd ..
