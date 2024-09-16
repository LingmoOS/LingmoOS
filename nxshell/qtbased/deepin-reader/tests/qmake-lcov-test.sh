#!/bin/bash

# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

workdir=. #编译路径

executable=test-deepin-reader #可执行程序的文件名

build_dir=$workdir

result_coverage_dir=$build_dir/coverage

result_report_dir=$build_dir/report/report.xml

$build_dir/$executable --gtest_output=xml:$result_report_dir

lcov -d $build_dir -c -o $build_dir/coverage.info

lcov --extract $build_dir/coverage.info '*/reader/*' -o  $build_dir/coverage.info

lcov --remove $build_dir/coverage.info '*/tests/*' -o  $build_dir/coverage.info

genhtml -o $result_coverage_dir $build_dir/coverage.info

exit 0
