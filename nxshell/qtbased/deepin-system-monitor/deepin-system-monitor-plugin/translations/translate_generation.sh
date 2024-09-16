#!/bin/bash
# this file is used to auto-generate .qm file from .ts file.
# author: shibowen at linuxdeepin.com

ts_list=(`ls *.ts`)

for ts in "${ts_list[@]}"
do
    printf "\nprocess ${ts}\n"
    /usr/lib/x86_64-linux-gnu/qt5/bin/lrelease "${ts}"
done
