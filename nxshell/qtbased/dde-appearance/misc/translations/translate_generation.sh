#!/bin/bash
# this file is used to auto-generate .qm file from .ts file.
# author: shibowen at linuxdeepin.com
CURRENT_DIR=$(pwd)
PROJECT_DIR=$CURRENT_DIR/misc/translations
cd ${PROJECT_DIR}
echo 当前目录：${CURRENT_DIR}
echo 工程目录：${PROJECT_DIR}

ts_list=(`ls translations/*.ts`)

for ts in "${ts_list[@]}"
do
    printf "\nprocess ${ts}\n"
    lrelease "${ts}"
done

cd ${CURRENT_DIR}
