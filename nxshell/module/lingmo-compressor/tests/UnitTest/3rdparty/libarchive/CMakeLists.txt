# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

#1.定义cmake版本
cmake_minimum_required(VERSION 3.9.5)
#2.设置项目名称
project(libarchive)

add_subdirectory(readonlylibarchiveplugin)

add_subdirectory(readwritelibarchiveplugin)
