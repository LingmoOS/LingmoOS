#!/bin/bash

BUILD_DIR=build
REPORT_DIR=report
PROJECT_REALNAME=dde-printer

rm -rf $BUILD_DIR
mkdir $BUILD_DIR
rm -f asan*

cd $BUILD_DIR
qmake CONFIG+=debug ../../src/cppcups
make

qmake CONFIG+=debug ../
make -j 8

lcov -c -i -d ./ -o init.info
../tests --gtest_output=xml:dde_test_report_${PROJECT_REALNAME}.xml
lcov -c -d ./ -o coverage.info
lcov -a init.info -a coverage.info -o total.info
lcov -r total.info "*/tests/*" "*/usr/include*"  "*.h" "*build/*" "*/dbus/*" -o final.info

mv asan_printer.log* asan_${PROJECT_REALNAME}.log

cd ..
rm -rf $REPORT_DIR
mkdir -p $REPORT_DIR
genhtml -o $REPORT_DIR ./build/final.info

# CI检查项目目录修改
BUILDUT_DIR=build-ut
[ -d ../$BUILDUT_DIR ] && rm -rf ../$BUILDUT_DIR
mkdir -p ../$BUILDUT_DIR
cp -r $REPORT_DIR ../$BUILDUT_DIR
cp $BUILD_DIR/asan_${PROJECT_REALNAME}.log ../$BUILDUT_DIR
cp $BUILD_DIR/dde_test_report_${PROJECT_REALNAME}.xml ../$BUILDUT_DIR

cd ../$BUILDUT_DIR
mv ./report ./html
cd html
mv ./index.html ./cov_dde-printer.html
chmod 755 cov_dde-printer.html

cd ..
mkdir report
mv dde_test_report_${PROJECT_REALNAME}.xml ./report/report_${PROJECT_REALNAME}.xml

