#!/bin/bash
# aspell-dicts <dict-list>
#
# <dict-list> is a text file containing the URI for one or more dicts i.e.:
# ftp://ftp.gnu.org/gnu/aspell/dict/bg/aspell5-bg-4.0-0.tar.bz2

usage() {
    echo  "Usage:"
    echo  "$0 <dict-list>"
}

if [ -z "$1" ]
    then
    usage
    exit 1
fi

i=0
for DICT in `cat $1`; do
    TGZ=`echo ${DICT} | sed "s|ftp://ftp.gnu.org/gnu/aspell/dict/.*/||"`
    wget ${DICT}
    tar xjf ${TGZ}
    cd `echo ${TGZ} | sed "s:.tar.bz2$::"`
    ./configure
    make install
    cd ..
    mv ${TGZ} dictionaries_pool
    rm -r `echo ${TGZ} | sed "s:.tar.bz2$::"`
    i=`expr $i + 1`
done
