#!/bin/bash
# this file is used to auto-generate .qm file from .ts file.
# author: shibowen at linuxdeepin.com

lupdate  src/ -ts -no-obsolete translations/*.ts

desk_ts_list=(`ls translations/desktop/*.ts`)
for ts in "${desk_ts_list[@]}"
do
    printf "\nprocess ${ts}\n"
    lrelease "${ts}"
done

ts_list=(`ls translations/*.ts`)

for ts in "${ts_list[@]}"
do
    printf "\nprocess ${ts}\n"
    lrelease "${ts}"
done