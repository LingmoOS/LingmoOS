#!/usr/bin/env sh

# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later

cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --target ut-dtkbluetooth -- -j $(nproc)
ctest --test-dir build -VV

lcov -d build/ -c -o build/coverage_all.info
lcov --remove build/coverage_all.info "*/tests/*" "*/usr/include*" "*/moc*.cpp" --output-file build/coverage.info
genhtml -o build/coverage_html build/coverage.info
