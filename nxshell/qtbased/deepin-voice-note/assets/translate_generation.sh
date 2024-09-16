#!/bin/bash
# this file is used to auto-generate .qm file from .ts file.
# Maintainer: V4fr3e <liujinli@uniontech.com>
# this script should only be called in cmake file.
# parameters:
#   $1 <translations dir>
#   $2 <update_trans>   true for update ts
#   $3 [source dir]
#   $4 [project name]

translations_dir=$1
need_update_trans=$2
source_dir=$3
project=$4
en_ts_file=${translations_dir}/${project}.ts

#check if need to update ts file
if echo "$need_update_trans" | grep -qwi "true"
then
    printf "\nBegin--> ${en_ts_file}\n"
    lupdate ${source_dir}  -ts ${en_ts_file}
fi

ts_list=(`ls ${translations_dir}/*.ts`)

for ts in "${ts_list[@]}"
do
    printf "\nprocess ${ts}\n"
    lrelease "${ts}"
done
