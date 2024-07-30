#!/bin/bash

if [ ! -f "contrib/run-krazy.sh" ];
then
    echo "This script needs to be started from KAMD's root directory"
    exit
fi

DIRS=$1

if [ ! -n "$1" ];
then
    DIRS="lib service utils workspace"
fi



echo $DIRS
CURRENT_DIRECTORY=$PWD

for dir in $DIRS;
do
    echo "Running krazy2 on $dir ..."
    cd $CURRENT_DIRECTORY/src/$dir && krazy2all --exclude license > /tmp/$dir.krazy
done
