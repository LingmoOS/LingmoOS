#!/bin/bash

set -e


# variables
projet_name='lingmo-notebook'
project_path=$(dirname `realpath $0`)
src_dir=${project_path}/..
translations_dir=$src_dir/translations/$projet_name
po_dir=$src_dir/po
generate_ui_dir=$src_dir/qt_ui
generate_pic_dir=$src_dir/src/pic
shell_dir=$src_dir/sh
locales=('en'
    'zh_CN'
    'bo_CN'
    'kk'
    'ky'
    'ug'
    'ru'
    'zh_HK'
    'mn'
    'de'
    'es'
    'fr'
    )

declare -A ts_files_head
ts_files_head[${translations_dir}/${projet_name}_]="\
${src_dir}/src"


# alias
RM='rm -rf'
MKDIR='mkdir'
CD='cd'
LUPDATE='lupdate -silent'
LRELEASE='lrelease -silent'
case `uname` in
    Linux)
        SED_I='sed -i '
    ;;
    Darwin)
        SED_I='sed -i "" '
    ;;
    *)
    ;;
esac


# fun
function clear_po()
{
    ${CD} ${src_dir}
    ${RM} ${po_dir}/*.ts \
          ${po_dir}/*.po \
          ${po_dir}/*.pot \
          ${po_dir}/.intltool-merge-cache
    find . -name ".cvsignore" | xargs ${RM}
    find . -name "Makefile.am" | xargs ${RM}
    ${CD} $OLDPWD
}

function ts_files_check()
{
    all_checked=0
    for locale in ${locales[@]}; do
        for ts_file in ${!ts_files_head[@]}; do
            if [ ! -f ${ts_file}${locale}.ts ]; then
                echo "File ${ts_file}${locale}.ts didn't exist!"
                all_checked=1
            fi
        done
    done
    return ${all_checked}
}
