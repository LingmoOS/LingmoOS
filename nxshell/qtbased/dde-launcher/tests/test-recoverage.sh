#!/bin/bash

BUILD_DIR=build
REPORT_DIR=report

cd ../
rm -rf $BUILD_DIR
mkdir $BUILD_DIR
cd $BUILD_DIR

cmake -DCMAKE_BUILD_TYPE=Debug ..
make -j 8

cd tests/

lcov -c -i -d ./ -o init.info
./dde_launcher_unit_test --gtest_output=xml:dde_test_report_dde_launcher.xml
lcov -c -d ./ -o cover.info
lcov -a init.info -a cover.info -o total.info
lcov -r total.info "*/tests/*" "*/usr/include*" "*build/src*" '*dbusinterface*' '*tests*' '*dbusservices*' -o final.info

rm -rf ../../tests/$REPORT_DIR
mkdir -p ../../tests/$REPORT_DIR
genhtml -o ../../tests/$REPORT_DIR final.info

mv asan.log* asan_dde-launcher.log
