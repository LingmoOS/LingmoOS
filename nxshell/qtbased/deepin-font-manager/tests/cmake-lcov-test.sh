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

./tests/font-manager-test --gtest_output=xml:./report/report.xml

mkdir -p report

lcov -d ../ -c -o ./report/coverage.info

lcov --extract ./report/coverage.info '*/deepin-font-manager/*' '*/deepin-font-preview-plugin/*' '*/libdeepin-font-manager/*' -o  ./report/coverage.info

lcov --remove ./report/coverage.info '*/tests/*' -o ./report/coverage.info

genhtml -o ./report ./report/coverage.info

exit 0