#!/bin/bash

# The script finds build directories for the current
# src directory and builds them
#
# For example, for the source dir:
#   /some/path/kde/src/project/whatever
# It finds:
#   /some/path/kde/build*/project/whatever

current_dir=`pwd`

all_root_dir=`pwd | sed 's#/src/.*##'`
src_root_dir=$all_root_dir/src

echo "src:   $src_root_dir"

for build_root_dir in $all_root_dir/build*; do
    echo "building in $build_root_dir"

    cd $current_dir
    current_dir_log=`OBJ_REPLACEMENT=s#$src_root_dir#$build_root_dir# makeobj`
    if [ "$?" = "0" ]
    then
        echo "... success"
    else
        echo "... FAILED"
        echo $current_dir_log
        exit
    fi

done

git commit

