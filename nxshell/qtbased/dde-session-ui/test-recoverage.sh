#!/bin/bash
#
# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

# 单元测试覆盖率统计脚本

BUILD_DIR=build
REPORT_DIR=report

# 工程列表
targets=()
targets[${#targets[*]}]=dde-bluetooth-dialog
targets[${#targets[*]}]=dde-license-dialog
targets[${#targets[*]}]=dde-lowpower
targets[${#targets[*]}]=dde-notification-plugin
targets[${#targets[*]}]=dde-osd
targets[${#targets[*]}]=dde-suspend-dialog
targets[${#targets[*]}]=dde-warning-dialog
targets[${#targets[*]}]=dde-welcome
targets[${#targets[*]}]=dde-wm-chooser
targets[${#targets[*]}]=dmemory-warning-dialog
targets[${#targets[*]}]=dnetwork-secret-dialog
targets[${#targets[*]}]=dde-touchscreen-dialog

# 编译源文件
rm -rf $BUILD_DIR
mkdir $BUILD_DIR
cd $BUILD_DIR

cmake -DCMAKE_BUILD_TYPE=Debug ..

# 多线程编译
cpunum=`cat /proc/cpuinfo  | grep "processor" | wc -l`
make -j $cpunum

for target in ${targets[*]}; do
    # 生成单元测试报告
    cd $target
    cd tests
    ./ut-${target} --gtest_output=xml:test_report_${target}.xml
    mv asan_${target}.log* asan_${target}.log
    cd ../..
done

# 单元测试覆盖率
lcov --directory . --capture --output-file ./coverage.info
lcov --remove ./coverage.info "*/tests/*" "*/usr/include*" "*build/src*" "*persistence.cpp*" "*notifications_dbus_adaptor.cpp*" "*notifysettings.cpp*" "*dbuslogin1manager.cpp*" "*persistence.h*" "*notifysettings.h*" "*dbuslogin1manager.h*" "*dbusdockinterface.h*" "*dbus_daemon_interface.h*" "*dbus_daemon_interface.cpp*" "*/global_util/dbus/*" "*/global_util/xkbparser*" "*icondata.h*" "*icondata.cpp*" "*kblayoutindicator.h*" "*kblayoutindicator.cpp*" -o ./coverage.info

# 生成html
genhtml -o tests/$REPORT_DIR coverage.info
