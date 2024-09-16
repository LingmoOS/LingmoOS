#!/bin/bash
BUILD_DIR=build-ut
HTML_DIR=html
REPORT_DIR=report
PROJECT_REALNAME=dde-session-shell

cd ../
rm -rf $BUILD_DIR
mkdir $BUILD_DIR

cd $BUILD_DIR
cmake -DCMAKE_BUILD_TYPE=Debug ..
make -j$(nproc)

cd tests/
lcov -c -i -d ./ -o init.info # 初始化并创建基准数据文件

dde-lock/dde-lock-test --gtest_output=xml:../$REPORT_DIR/ut-report_dde-lock.xml # 执行编译后的测试文件并生成测试用例数据报告
mv asan.log* ../asan_dde-lock.log # 收集 dde-lock 内存泄漏数据
lcov -c -d dde-lock/ -o coverage_dde-lock.info # 收集 dde-lock 单元测试数据

lightdm-deepin-greeter/lightdm-deepin-greeter-test --gtest_output=xml:../$REPORT_DIR/ut-report_lightdm-deepin-greeter.xml # 执行编译后的测试文件并生成测试用例数据报告
mv asan.log* ../asan_lightdm-deepin-greeter.log # 收集 lightdm-deepin-greeter 内存泄漏数据
lcov -c -d lightdm-deepin-greeter/ -o coverage_lightdm-deepin-greeter.info # 收集 lightdm-deepin-greeter 单元测试数据

lcov -a init.info -a coverage_dde-lock.info -a coverage_lightdm-deepin-greeter.info -o coverage_total.info # 将单元测试数据汇总
lcov -r coverage_total.info "*/tests/*" "*/usr/include*"  "*.h" "*build/*" "*/dbus/*" "*/xcb/*" -o final.info # 过滤不需要的信息

genhtml -o ../$HTML_DIR --title $PROJECT_REALNAME final.info # 将最终的单元测试覆盖率数据生成 html 文件

mv ../$HTML_DIR/index.html ../$HTML_DIR/cov_dde-session-shell.html


