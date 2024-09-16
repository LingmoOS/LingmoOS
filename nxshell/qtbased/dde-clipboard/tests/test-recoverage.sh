#!/bin/bash

BUILD_DIR=build-ut
HTML_DIR=html
REPORT_DIR=report

cd ../
rm -rf $BUILD_DIR
mkdir $BUILD_DIR
cd $BUILD_DIR

cmake -DCMAKE_BUILD_TYPE=Debug ..
make -j 8

mkdir -p ../tests/$HTML_DIR
mkdir -p ../tests/$REPORT_DIR

lcov -c -i -d ./ -o init.info
./ut-dde-clipboard --gtest_output=xml:./$REPORT_DIR/ut-report_dde-clipboard.xml
lcov -c -d ./ -o cover.info
lcov -a init.info -a cover.info -o total.info
lcov -r total.info "*/tests/*" "*/usr/include*" '*tests*' -o final.info

genhtml -o ./$HTML_DIR final.info

mv ./$HTML_DIR/index.html ./$HTML_DIR/cov_dde-clipboard.html

mv asan_board.log* asan_dde-clipboard.log