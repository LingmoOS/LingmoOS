#!/bin/bash

# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

#workdir=$(cd ../../$(dirname $0)/build-filemanager-unknown-Debug/test; pwd)

workdir=.

executable=test-deepin-reader

build_dir=$workdir

result_coverage_dir=$build_dir/coverage

result_report_dir=$build_dir/report/report.xml

#下面二选1进行操作，一种正向操作，一种逆向操作

extract_info="*/deepin-reader/*"

remove_info="*/tests/*"

$build_dir/$executable --gtest_output=xml:$result_report_dir

lcov -d $build_dir -c -o $build_dir/coverage.info

lcov --remove $build_dir/coverage.info $remove_info --output-file $build_dir/coverage.info

lcov --extract $build_dir/coverage.info $extract_info --output-file  $build_dir/coverage.info

lcov --list-full-path -e $build_dir/coverage.info –o $build_dir/coverage-stripped.info

genhtml -o $result_coverage_dir $build_dir/coverage.info

nohup x-www-browser $result_coverage_dir/index.html &

nohup x-www-browser $result_report_dir &

lcov -d $build_dir –z

exit 0
