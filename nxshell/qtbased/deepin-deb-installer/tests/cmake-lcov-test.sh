# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: CC0-1.0

utdir=build-ut
rm -r $utdir
rm -r ../$utdir
mkdir ../$utdir
cd ../$utdir

cmake -DCMAKE_BUILD_TYPE=Debug ..
make -j16

./tests/deepin-deb-installer-test --gtest_output=xml:./report/report.xml

workdir=$(cd ../$(dirname $0)/$utdir; pwd)

mkdir -p report

lcov -d $workdir -c -o ./report/coverage.info

lcov --extract ./report/coverage.info '*/src/*' -o ./report/coverage.info

lcov --remove ./report/coverage.info '*/tests/*' '*/process/*' -o ./report/coverage.info

genhtml -o ./report ./report/coverage.info

exit 0
