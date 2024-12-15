#!/bin/bash

# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later
SCRIPT_PATH=$(dirname $0)
cd $SCRIPT_PATH && cd ..

BUILD_DIR=$PWD/build
HTML_DIR=${BUILD_DIR}/html
XML_DIR=${BUILD_DIR}/report

export ASAN_OPTIONS="halt_on_error=0"

cmake -B${BUILD_DIR} -DCMAKE_BUILD_TYPE=Debug -DENABLE_COV=ON

cmake --build ${BUILD_DIR} --target unit-tests -j$(nproc)

cd $BUILD_DIR

tests/unit-tests --gtest_output=xml:${XML_DIR}/report_qtintegration.xml

lcov -d ./ -c -o coverage_all.info
lcov --remove coverage_all.info "*/tests/*" "*/usr/include*" "*build/*" --output-file coverage.info
genhtml -o $HTML_DIR $BUILD_DIR/coverage.info && mv ${BUILD_DIR}/html/index.html ${BUILD_DIR}/html/cov_qtintegration.html
