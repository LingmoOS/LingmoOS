#!/bin/bash

set -e

project_path=$(dirname `realpath $0`)
source ${project_path}/common.sh

function update_ts()
{
    for locale in ${locales[@]}; do
        for ts_file in ${!ts_files_head[@]}; do
            ${LUPDATE} -target-language ${locale} ${ts_files_head[${ts_file}]} -ts ${ts_file}${locale}.ts
        done
    done
}

update_ts

if [ ! -f "${po_dir}/POTFILES.in" ]; then
    echo "${po_dir}/POTFILES.in didn't exist, shell will exec generate_translations.sh."
    
    bash ${shell_dir}/generate_translations.sh
    exit 0
fi

cd $src_dir

for locale in ${locales[@]}
do
    lconvert -i ${translations_dir}/${projet_name}_${locale}.ts \
             -o ${po_dir}/${locale}.po 
done

cd $po_dir
for locale in ${locales[@]}; do
    intltool-update  $locale
done

cd $src_dir
for locale in ${locales[@]}; do
    lconvert -no-obsolete -i ${po_dir}/${locale}.po -o ${po_dir}/${locale}.ts
    
    lconvert -i ${translations_dir}/${projet_name}_${locale}.ts ${po_dir}/${locale}.ts \
             -o ${translations_dir}/${projet_name}_${locale}.ts
done

clear_po