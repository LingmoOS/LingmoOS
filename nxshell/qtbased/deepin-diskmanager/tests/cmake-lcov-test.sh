#!/bin/bash

# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-only

export DISPLAY=:0
echo "Uos123!!" | sudo xhost +

utdir=build-ut
#echo "Uos123!!" | sudo -S rm -r $utdir
#echo "Uos123!!" | sudo -S rm -r ../$utdir
mkdir -p ../$utdir
cd ../$utdir

cmake ..
make -j4

touch ./tests/utcase.log
echo "Uos123!!" | sudo -S ./tests/deepin-diskmanager_app_test --gtest_output=xml:./ut-report.txt

workdir=$(cd ../$(dirname $0)/$utdir; pwd)

mkdir -p report

echo "Uos123!!" | sudo -S lcov -d $workdir -c -o ./report/coverage.info

echo "Uos123!!" | sudo -S lcov --extract ./report/coverage.info '*/application/*' '*/basestruct/*' -o ./report/coverage.info

echo "Uos123!!" | sudo -S lcov --remove ./report/coverage.info '*/tests/*' '*/application/partedproxy/*' -o ./report/coverage.info

echo "Uos123!!" | sudo -S genhtml -o ./report ./report/coverage.info

echo "Uos123!!" | sudo -S chmod 666 ./ut-report.txt

echo "Uos123!!" | sudo -S chmod 666 ./report/index.html

exit 0
