# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: CC0-1.0

cmake_minimum_required(VERSION 3.10)
project(lingmo-ocr)

option(DOTEST "option for test" OFF)

#lingmo-ocr
add_subdirectory(src)
