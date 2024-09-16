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
######################
export DISPLAY=:0.0
export XDG_CURRENT_DESKTOP=Deepin
export QT_IM_MODULE=fcitx
cmake -DCMAKE_SAFETYTEST_ARG="CMAKE_SAFETYTEST_ARG_ON" .. -DCMAKE_INSTALL_PREFIX="/usr"
make -j4

./tests/deepin-fcitxconfigtool-plugin_test > asan_deepin-fcitxconfigtool-plugin.log 2>&1
######################
cd -
echo "Uos123!!" | sudo -S mv ./build/asan.log.* ./build-ut/interlog-deepin-fcitxconfigtool-plugin.log
echo "Uos123!!" | sudo -S chmod 777 ./build-ut/asan_deepin-fcitxconfigtool-plugin.log

# UT for index.html and ut-report.txt
cd tests
sh cmake-lcov-test.sh 
cd -
cp build-ut/report/index.html build-ut/html/cov_deepin-fcitxconfigtool-plugin.html
cp build-ut/ut-report.txt build-ut/report/report_deepin-fcitxconfigtool-plugin.xml
