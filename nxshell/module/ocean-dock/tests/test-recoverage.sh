#!/bin/bash

BUILD_DIR=build-ut
HTML_DIR=html
REPORT_DIR=report

cd ../
rm -rf $BUILD_DIR
mkdir $BUILD_DIR
cd $BUILD_DIR

cmake -DCMAKE_BUILD_TYPE=Debug ..

cmake ../
make -j 16

cd tests/

./ocean_dock_unit_test --gtest_output=xml:../$REPORT_DIR/ut-report_ocean_dock.xml
lcov -c -d ./ -o cover.info
lcov -e cover.info '*/frame/*' '*/ocean-dock/widgets/*' -o code.info
lcov -r code.info '*/dbus/*' '*/xcb/*' -o final.info


genhtml -o ../$HTML_DIR final.info
mv ../$HTML_DIR/index.html ../$HTML_DIR/cov_ocean-dock.html

mv asan.log* ../asan_ocean-dock.log
