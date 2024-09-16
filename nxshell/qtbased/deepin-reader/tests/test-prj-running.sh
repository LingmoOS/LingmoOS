#!/bin/bash

# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

builddir=build
reportdir=build-ut
rm -rf $builddir
rm -rf ../$builddir
rm -rf $reportdir
rm -rf ../$reportdir
mkdir ../$builddir
mkdir ../$reportdir
cd ../$builddir
#编译
qmake DEFINES+=CMAKE_SAFETYTEST_ARG_ON ../deepin_reader.pro
make -j8
#生成asan日志和ut测试xml结果
./tests/test-deepin-reader --gtest_output=xml:./report/report_deepin-reader.xml

workdir=$(cd ../$(dirname $0)/$builddir; pwd)

mkdir -p report
#统计代码覆盖率并生成html报告
lcov -d $workdir -c -o ./coverage.info

lcov --extract ./coverage.info '*/reader/*' -o  ./coverage.info

lcov --remove ./coverage.info '*/tests/*' -o  ./coverage.info

genhtml -o ./html ./coverage.info

mv ./html/index.html ./html/cov_deepin-reader.html
#对asan、ut、代码覆盖率结果收集至指定文件夹
cp -r html ../$reportdir/
cp -r report ../$reportdir/
cp -r asan*.log* ../$reportdir/asan_deepin-reader.log

exit 0
