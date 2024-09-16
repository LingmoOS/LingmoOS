#!/bin/bash

# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later

cd "$(git rev-parse --show-toplevel)" || exit 255

BUILD_DIR=${BUILD_DIR:="build-cov"}

HTML_DIR=${BUILD_DIR}/html

export ASAN_OPTIONS="halt_on_error=0"

cmake -B "$BUILD_DIR" \
	-DCMAKE_BUILD_TYPE=Debug \
	-DCMAKE_CXX_FLAGS="--coverage -fsanitize=address -fsanitize-recover=address " \
	-DCMAKE_CXX_LINK_FLAGS="-lasan"

cmake --build "$BUILD_DIR" -j$(nproc)

cmake --build "$BUILD_DIR" -j$(nproc) -t test

gcovr -f "src/*" --html-details "$HTML_DIR"/coverage.html
