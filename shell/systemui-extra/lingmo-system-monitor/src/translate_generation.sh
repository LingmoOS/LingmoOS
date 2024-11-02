#!/bin/bash

ts_list=(`ls translation/*.ts`)
source /etc/os-release
version=(`echo $ID`)

for ts in "${ts_list[@]}"
do
    printf "\nprocess ${ts}\n"
    if [ "$version" == "fedora" ] || [ "$version" == "opensuse-leap" ] || [ "$version" == "opensuse-tumbleweed" ];then
        lrelease-qt5 "${ts}"
    else
	lrelease "${ts}"
    fi
done
