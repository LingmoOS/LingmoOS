#!/bin/bash

# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later

TESTS_TARGET=unit-test
filter_files=(
 "QtCore/*"
 "QtGui/*"
 "QtQml/*"
 "QtQuick/*"
 "*dquickbusyindicator*"
 "*dquicksystempalette*"
)

SHELL_FOLDER=$(dirname $(readlink -f "$0"))

# project directroy
SOURCE_DIR=${SHELL_FOLDER}/../
BUILD_DIR=${SOURCE_DIR}/build
TESTS_BUILD_DIR=${BUILD_DIR}/tests
HTML_DIR=${TESTS_BUILD_DIR}/html

export ASAN_OPTIONS="halt_on_error=0"

cd ${SOURCE_DIR}

cmake -B${BUILD_DIR} -DCMAKE_BUILD_TYPE=Debug -GNinja -DENABLE_COV=ON

cmake --build ${BUILD_DIR} --target ${TESTS_TARGET}

cd ${BUILD_DIR}

${TESTS_BUILD_DIR}/${TESTS_TARGET}
lcov -d ./ -c -o coverage_all.info

lcov --remove coverage_all.info "*/tests/*" "/usr/include*" "*build/*" ${filter_files[*]} --output-file coverage.info

cd ${BUILD_DIR}
genhtml -o ${HTML_DIR} ${BUILD_DIR}/coverage.info && ln -sf ${HTML_DIR}/index.html ${BUILD_DIR}/cov_index.html

test -e ${TESTS_BUILD_DIR}/asan.log* && mv ${TESTS_BUILD_DIR}/asan.log* ${BUILD_DIR}/asan_index.log
