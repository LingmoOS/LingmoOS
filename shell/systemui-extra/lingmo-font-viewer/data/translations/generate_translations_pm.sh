#! /bin/bash

ts_file_list=(`ls data/translations/*.ts`)

for ts in "${ts_file_list[@]}"
do
	lrelease "${ts}"
done
