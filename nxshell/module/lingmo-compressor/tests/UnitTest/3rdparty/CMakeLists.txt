# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

#1.定义cmake版本
cmake_minimum_required(VERSION 3.9.5)
#2.设置项目名称
set(EXE_NAME 3rdparty)
project(${EXE_NAME}_test)

#ADD_COMPILE_OPTIONS(-fno-access-control)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")


set(3RDPARTYRELATIVEPATH ../../../../3rdparty) #相对于源码路径

add_subdirectory(interface)

add_subdirectory(cli7zplugin)

add_subdirectory(clirarplugin)

add_subdirectory(libarchive)

add_subdirectory(libsinglefileplugin)

add_subdirectory(libpigzplugin)

add_subdirectory(libzipplugin)

add_subdirectory(libminizipplugin)

##add_subdirectory(cliunarchiverplugin) #不可用

##add_subdirectory(clizipplugin) #不可用
