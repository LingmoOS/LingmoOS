# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

utdir=build-ut
rm -r $utdir
rm -r ../$utdir
mkdir ../$utdir

cd ../$utdir

cmake -DCMAKE_BUILD_TYPE=Debug ..
make -j16

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

workdir=$(cd ../$(dirname $0)/$utdir; pwd)

mkdir -p report
lcov -d $workdir -c -o ./report/coverage.info

#只收集src、3rdparty部分目录
lcov --extract ./report/coverage.info '*/src/*' '*/3rdparty/*' -o ./report/coverage.info
#过滤 tests 3rdparty/ChardetDetector目录
lcov --remove ./report/coverage.info '*/tests/*' '*/ChardetDetector/*' '*/interface/plugin/*' '*/interface/process/*' -o ./report/coverage.info

genhtml -o ./report ./report/coverage.info

exit 0
