#!/bin/bash

# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

builddir=build
reportdir=build-ut
rm -r $builddir
rm -r ../$builddir
rm -r $reportdir
rm -r ../$reportdir
mkdir ../$builddir
mkdir ../$reportdir
cd ../$builddir
#编译
cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_SAFETYTEST_ARG="CMAKE_SAFETYTEST_ARG_ON" ..
make -j8
#生成asan日志和ut测试xml结果
./bin/tests/bz2plugin_test --gtest_output=xml:./report/report_bz2plugin.xml
./bin/tests/cli7zplugin_test --gtest_output=xml:./report/report_cli7zplugin.xml
./bin/tests/clirarplugin_test --gtest_output=xml:./report/report_clirarplugin.xml
./bin/tests/deepin-compressor_test --gtest_output=xml:./report/report_deepin-compressor.xml
./bin/tests/gzplugin_test --gtest_output=xml:./report/report_gzplugin.xml
./bin/tests/interface_test --gtest_output=xml:./report/report_interface.xml
./bin/tests/libminizipplugin_test --gtest_output=xml:./report/report_libminizipplugin.xml
./bin/tests/libzipplugin_test --gtest_output=xml:./report/report_libzipplugin.xml
./bin/tests/libpigzplugin_test --gtest_output=xml:./report/report_libpigzplugin.xml
./bin/tests/readonlylibarchiveplugin_test --gtest_output=xml:./report/report_readonlylibarchiveplugin.xml
./bin/tests/readwritelibarchiveplugin_test --gtest_output=xml:./report/report_readwritelibarchiveplugin.xml
./bin/tests/xzplugin_test --gtest_output=xml:./report/report_xzplugin.xml

workdir=$(cd ../$(dirname $0)/$builddir; pwd)

mkdir -p report
#统计代码覆盖率并生成html报告
lcov -d $workdir -c -o ./coverage.info

lcov --extract ./coverage.info '*/src/*' -o ./coverage.info

lcov --remove ./coverage.info '*/tests/*' -o ./coverage.info

genhtml -o ./html ./coverage.info

mv ./html/index.html ./html/cov_deepin-compressor.html
#对asan、ut、代码覆盖率结果收集至指定文件夹
cp -r html ../$reportdir/
cp -r report ../$reportdir/
cp -r asan*.log* ../$reportdir/asan_deepin-compressor.log

exit 0
