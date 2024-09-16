# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: CC0-1.0

#!/bin/bash
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
./tests/deepin-deb-installer-test --gtest_output=xml:./report/report_deepin-deb-installer.xml

workdir=$(cd ../$(dirname $0)/$builddir; pwd)

mkdir -p report
#统计代码覆盖率并生成html报告
lcov -d $workdir -c -o ./coverage.info

lcov --extract ./coverage.info '*/src/*' -o ./coverage.info

lcov --remove ./coverage.info '*/tests/*' '*/process/*' -o ./coverage.info

genhtml -o ./html ./coverage.info

mv ./html/index.html ./html/cov_deepin-deb-installer.html
#对asan、ut、代码覆盖率结果收集至指定文件夹
cp -r html ../$reportdir/
cp -r report ../$reportdir/
cp -r asan*.log* ../$reportdir/asan_deepin-deb-installer.log

exit 0
