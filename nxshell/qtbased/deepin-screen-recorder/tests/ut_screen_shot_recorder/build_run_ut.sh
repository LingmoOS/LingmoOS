#!/bin/bash

# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

export DISPLAY=":0"
export QT_QPA_PLATFORM=

export QT_LOGGING_RULES="qt.qpa.xcb.xcberror=false"
export QT_LOGGING_RULES="qt.qpa.xcb.*=false"
export QT_LOGGING_RULES="qt.qpa.*=false"
export QT_LOGGING_RULES="*=false"

rm -rf ./ut_screen_shot_recorder/build-ut
mkdir ./ut_screen_shot_recorder/build-ut

cd ./ut_screen_shot_recorder/build-ut
export QT_SELECT=qt5
qmake ../
make -j4

#workdir=$(cd ../../$(dirname $0)/build-filemanager-unknown-Debug/test; pwd)
workdir=$(cd ../../$(dirname $0)/build-ut; pwd)
executable=ut_screen_shot_recorder #可执行程序的文件名

#下面是覆盖率目录操作，一种正向操作，一种逆向操作
extract_info="*/deepin-screen-recorder/*" #针对当前目录进行覆盖率操作
remove_info="*tests* *build-ut* *3rdparty*" #排除当前目录进行覆盖率操作


build_dir=$workdir
result_coverage_dir=$build_dir/html
result_report_dir=$build_dir/report/report_ut_screen_shot_recorder.xml

ASAN_OPTIONS="fast_unwind_on_malloc=1"
$build_dir/$executable --gtest_output=xml:$result_report_dir

lcov -d $build_dir -c -o $build_dir/coverage.info

lcov --extract $build_dir/coverage.info $extract_info --output-file  $build_dir/coverage.info
lcov --remove $build_dir/coverage.info $remove_info --output-file $build_dir/coverage.info

lcov --list-full-path -e $build_dir/coverage.info –o $build_dir/coverage-stripped.info

genhtml -o $result_coverage_dir $build_dir/coverage.info

cp $build_dir/report/report_ut_screen_shot_recorder.xml ../../../build-ut/report/report_ut_screen_shot_recorder.xml
cp $build_dir/html/index.html ../../../build-ut/html/cov_ut_screen_shot_recorder.html
cp $build_dir/asan_ut_screen_shot_recorder.log.* ../../../build-ut/asan_ut_screen_shot_recorder.log  

#nohup x-www-browser $result_coverage_dir/index.html &
#nohup x-www-browser $result_report_dir &
 
#lcov -d $build_dir –z
exit 0