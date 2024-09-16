#!/bin/env bash

# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later

cd "$(git rev-parse --show-toplevel)" || exit 255

BUILD_DIR=${BUILD_DIR:="build-prof"}

cmake -B "$BUILD_DIR" \
    -DCMAKE_BUILD_TYPE=RelWithDebInfo \
    -DPROFILING_MODE=ON

cmake --build "$BUILD_DIR" -j$(nproc)

QT_LOGGING_RULES="*.debug=false;*.info=false;*.warning=false" valgrind --tool=cachegrind --cachegrind-out-file="$BUILD_DIR/profiling.out" "$BUILD_DIR/apps/dde-application-manager/src/dde-application-manager"

echo "you can use Kcachegrind to check this profiling result."
