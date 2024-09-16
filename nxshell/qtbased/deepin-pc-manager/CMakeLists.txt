# SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

cmake_minimum_required(VERSION 3.13)

# If do't define version number, specify the version number
set(VERSION "1.0.0" CACHE STRING "define project version")

# project 有两种写法，这里建议使用这种写法，补全信息有助于后续使用
project(deepin-pc-manager
  LANGUAGES CXX C
  HOMEPAGE_URL https://github.com/linuxdeepin/deepin-pc-manager
  DESCRIPTION "deepin pc manager."
  VERSION ${VERSION}
)

# 定义GNU标准安装目录，使用此定义可兼容不同发行版之间的安装目录差异
include(GNUInstallDirs)
# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
# 设置为检查C++标准打开，若未设置CMAKE_CXX_STANDARD，则会报错
set(CMAKE_CXX_STANDARD_REQUIRED on)
# 若使用标准C++开发，则不需要以下两行，使用Qt则需要
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
# 打开所有编译警告，理论上项目中不允许有编译警告
# 在外部添加编译参数的方法，例：cmake -DCMAKE_CXX_FLAGS="-Wl,--as-needed"
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

# 安全编译选项
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -z relro -z now -z noexecstack -pie -fstack-protector-all")
# 生成编译命令，用于支持clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 若未指定安装前缀，则指定前缀为/usr
# 注意大部分发行版默认会将前缀设置为/usr/local，所以若需要安装到/usr，还需在编译时指定
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX /usr)
endif()

# 若未指定编译类型，则指定为Release，防止某些发行版不指定编译类型
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose Debug or Release" FORCE)
endif()

# 版本号宏定义
add_compile_definitions(QMAKE_VERSION="${VERSION}")

add_subdirectory(deepin-pc-manager)
add_subdirectory(deepin-pc-manager-server)
