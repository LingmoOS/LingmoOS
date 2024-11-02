#!/bin/bash

set -e

project_path=$(dirname `realpath $0`)
source ${project_path}/common.sh

if [ ! -d ${generate_ui_dir} ]; then
    ${MKDIR} ${generate_ui_dir}
fi


ui_file_list=`find ${src_dir} -path "${src_dir}/.pc"  -prune -o -name *.ui -print`

for ui_file in ${ui_file_list[@]}; do
    uic ${ui_file} -o ${generate_ui_dir}/ui_$(basename $ui_file .ui).h
done