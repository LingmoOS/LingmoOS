# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: CC0-1.0

cmake_minimum_required(VERSION 3.10)
project(deepin-ocr)

option(DOTEST "option for test" OFF)

#deepin-ocr
add_subdirectory(src)
