#!/bin/bash
BUILD_DIR=build-ut
HTML_DIR=html
REPORT_DIR=report
PROJECT_REALNAME=ocean-session-shell

cd ../
rm -rf $BUILD_DIR
mkdir $BUILD_DIR

cd $BUILD_DIR
cmake -DCMAKE_BUILD_TYPE=Debug ..
make -j$(nproc)

cd tests/
lcov -c -i -d ./ -o init.info # 初始化并创建基准数据文件

ocean-lock/ocean-lock-test --gtest_output=xml:../$REPORT_DIR/ut-report_ocean-lock.xml # 执行编译后的测试文件并生成测试用例数据报告
mv asan.log* ../asan_ocean-lock.log # 收集 ocean-lock 内存泄漏数据
lcov -c -d ocean-lock/ -o coverage_ocean-lock.info # 收集 ocean-lock 单元测试数据

lightdm-lingmo-greeter/lightdm-lingmo-greeter-test --gtest_output=xml:../$REPORT_DIR/ut-report_lightdm-lingmo-greeter.xml # 执行编译后的测试文件并生成测试用例数据报告
mv asan.log* ../asan_lightdm-lingmo-greeter.log # 收集 lightdm-lingmo-greeter 内存泄漏数据
lcov -c -d lightdm-lingmo-greeter/ -o coverage_lightdm-lingmo-greeter.info # 收集 lightdm-lingmo-greeter 单元测试数据

lcov -a init.info -a coverage_ocean-lock.info -a coverage_lightdm-lingmo-greeter.info -o coverage_total.info # 将单元测试数据汇总
lcov -r coverage_total.info "*/tests/*" "*/usr/include*"  "*.h" "*build/*" "*/dbus/*" "*/xcb/*" -o final.info # 过滤不需要的信息

genhtml -o ../$HTML_DIR --title $PROJECT_REALNAME final.info # 将最终的单元测试覆盖率数据生成 html 文件

mv ../$HTML_DIR/index.html ../$HTML_DIR/cov_ocean-session-shell.html


