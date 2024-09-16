#!/bin/bash
# this file is used to auto-generate .qm file from .ts file.
# author: shibowen at linuxdeepin.com

cd $(dirname $0)

ts_list=(`ls translations/*.ts`)
lupdate  -no-obsolete Printer.pro -ts translations/dde-printer.ts

for ts in "${ts_list[@]}"
do
    printf "\nprocess ${ts}\n"
    lrelease "${ts}"
done
