#!/bin/bash

# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

mkdir -p build
cd build
cmake .. -DCMAKE_C_COMPILER=/opt/llvm/bin/clang -DCMAKE_CXX_COMPILER=/opt/llvm/bin/clang++
make