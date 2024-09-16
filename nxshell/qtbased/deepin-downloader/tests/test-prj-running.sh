#!/bin/sh

# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

cd ../
[ -d build-ut ] && rm -fr build-ut 
mkdir -p build-ut/html
mkdir -p build-ut/report
[ -d build ] && rm -fr build
mkdir -p build

cd build/

# Sanitize for asan.log
export DISPLAY=:0.0
export XDG_CURRENT_DESKTOP=Deepin
export QT_IM_MODULE=fcitx

cmake -DCMAKE_SAFETYTEST_ARG="CMAKE_SAFETYTEST_ARG_ON" ..
make -j4

./bin/downloader_test > interlog_downloadmanager.log 2>&1
cd -
mv ./build/asan.log.* ./build-ut/asan_downloadmanager.log

# UT for index.html and ut-report.txt
cd tests
sh cmake-lcov-test.sh 
cd -
cp build-ut/report/index.html build-ut/html/cov_downloadmanager.html
cp build-ut/ut-report.txt build-ut/report/report_downloadmanager.xml
