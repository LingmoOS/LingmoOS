#!/bin/bash
#update ts or qm
#If you enter ts, update ts, otherwise update qm 

if [ "$1" = "ts" ]
then
    #1. update ts
    echo "update ts"
    lupdate ./lingmo-font-manager ./lingmo-font-preview-plugin ./liblingmo-font-manager -ts ./translations/*.ts -no-obsolete 
else
    #2. update qm
    echo "update qm"
    lrelease ./translations/*.ts
fi

read -s -t 2 -p "Please input Enter! " 
