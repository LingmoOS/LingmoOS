#!/bin/bash

# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

export DISPLAY=":0"
export QT_QPA_PLATFORM=

rm -rf ./ut_dde_dock_plugins/ut_record_time/build-ut
mkdir ./ut_dde_dock_plugins/ut_record_time/build-ut

cd ./ut_dde_dock_plugins/ut_record_time/build-ut
pwd
export QT_SELECT=qt5
qmake ../
make -j4

workdir=$(cd ../$(dirname $0)/build-ut; pwd)
executable=ut_record_time #可执行程序的文件名

#下面是覆盖率目录操作，一种正向操作，一种逆向操作
extract_info="*/dde-dock-plugins/*" #针对当前目录进行覆盖率操作
remove_info="*tests* *build-ut*" #排除当前目录进行覆盖率操作

build_dir=$workdir
result_coverage_dir=$build_dir/html
result_report_dir=$build_dir/report/report_ut_record_time.xml
ASAN_OPTIONS="fast_unwind_on_malloc=1"
$build_dir/$executable --gtest_output=xml:$result_report_dir

lcov -d $build_dir -c -o $build_dir/coverage.info

lcov --extract $build_dir/coverage.info $extract_info --output-file  $build_dir/coverage.info
lcov --remove $build_dir/coverage.info $remove_info --output-file $build_dir/coverage.info

lcov --list-full-path -e $build_dir/coverage.info –o $build_dir/coverage-stripped.info

genhtml -o $result_coverage_dir $build_dir/coverage.info

cp $build_dir/report/report_ut_record_time.xml ../../../../build-ut/report/report_ut_record_time.xml
cp $build_dir/html/index.html ../../../../build-ut/html/cov_ut_record_time.html
cp $build_dir/asan_ut_record_time.log.* ../../../../build-ut/asan_ut_record_time.log  
exit 0