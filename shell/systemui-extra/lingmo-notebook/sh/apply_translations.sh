#!/bin/bash

set -e

project_path=$(dirname `realpath $0`)
source ${project_path}/common.sh

ts_files_check
if [ $? != 0 ]; then
    echo 'ts_files_check failed!'
    exit 1
fi

${CD} $translations_dir

for locale in ${locales[@]}; do
    for ts_file in ${!ts_files_head[@]}; do
        ${LRELEASE} ${ts_file}${locale}.ts
    done
done

if [ ! -f "${po_dir}/POTFILES.in" ]; then
    echo "${po_dir}/POTFILES.in didn't exist! Except qm files, other files will not be generated."
    exit 1
fi

${CD} $src_dir
for locale in ${locales[@]}; do
    lconvert -i ${translations_dir}/${projet_name}_${locale}.ts -o ${po_dir}/${locale}.po 
done

file_list=`sed '/^#.*\|^$/d' ${po_dir}/POTFILES.in`
for file in ${file_list[@]}; do
    intltool-merge ${po_dir} -d -u -c ${po_dir}/.intltool-merge-cache ${src_dir}/${file} ${src_dir}/${file%.*}
done


clear_po
