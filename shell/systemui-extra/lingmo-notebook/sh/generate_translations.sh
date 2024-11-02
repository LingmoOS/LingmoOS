#!/bin/bash

set -e

project_path=$(dirname `realpath $0`)
source ${project_path}/common.sh

${CD} $src_dir

if [ -d ${po_dir} ]; then
    if [ "`ls -A ${po_dir}`" != "" ]; then
        echo "directory ${po_dir} exists! shell exit!"
        exit 1
    fi
else
    ${MKDIR} $po_dir
fi

intltool-prepare

${CD} $po_dir

intltool-update --pot --gettext-package=${projet_name}

${SED_I} "/\"Content-Type: text\/plain; charset=CHARSET\\\n\"/s/CHARSET/UTF-8/g" ${projet_name}.pot

for locale in ${locales[@]}
do 
    xgettext -i ${projet_name}.pot -o ${locale}.po --from-code=UTF-8 --omit-header -Lpo

    lconvert -i ${po_dir}/${locale}.po -o ${po_dir}/${locale}.ts
    
    lconvert -i ${translations_dir}/${projet_name}_${locale}.ts ${po_dir}/${locale}.ts \
             -o ${translations_dir}/${projet_name}_${locale}.ts
done

clear_po
${CD} $src_dir
find . -name "*.desktop" | xargs ${RM}