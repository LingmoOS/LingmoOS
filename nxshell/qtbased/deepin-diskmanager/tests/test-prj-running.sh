#!/bin/sh

# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-only

cd ../
[ -d build-ut ] && rm -fr build-ut 
mkdir -p build-ut/html
mkdir -p build-ut/report
[ -d build ] && rm -fr build
mkdir -p build

cd build/

# Sanitize for asan.log
######################
export DISPLAY=:0.0
xhost +
export XDG_CURRENT_DESKTOP=Deepin
export QT_IM_MODULE=fcitx
cmake -DCMAKE_SAFETYTEST_ARG="CMAKE_SAFETYTEST_ARG_ON" ..
make -j4
echo "Uos123!!" | sudo -S ./tests/deepin-diskmanager_app_test > asan_deepin-diskmanager.log 2>&1
######################
cd -
echo "Uos123!!" | sudo -S cp ./build/asan.log.* ./build-ut/asan_deepin-diskmanager.log
echo "Uos123!!" | sudo -S chmod 777 ./build-ut/asan_deepin-diskmanager.log

# UT for index.html and ut-report.txt
cd tests
sh cmake-lcov-test.sh 
cd -
cp build-ut/report/index.html build-ut/html/cov_deepin-diskmanager.html
cp build-ut/ut-report.txt build-ut/report/report_deepin-diskmanager.xml
