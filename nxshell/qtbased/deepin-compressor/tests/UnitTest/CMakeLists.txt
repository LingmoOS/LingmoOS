# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

#1.定义cmake版本
cmake_minimum_required(VERSION 3.9.5)
#2.设置项目名称
#访问私有方法 -fno-access-control
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage -fno-access-control -fno-inline")

#3 设置可执行程序输出目录
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin/tests)

if(${CMAKE_BUILD_TYPE} MATCHES "Debug")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -Wall -g -ggdb -Wl,-rpath=\"${LIBRARY_OUTPUT_PATH}\":/usr/lib/deepin-compressor/plugins")
else()
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wl,-O1,-rpath=\"${LIBRARY_OUTPUT_PATH}\":/usr/lib/deepin-compressor/plugins")
endif()

set(CMAKE_SAFETYTEST "${CMAKE_SAFETYTEST_ARG}")

if(CMAKE_SAFETYTEST STREQUAL "")
    set(CMAKE_SAFETYTEST "CMAKE_SAFETYTEST_ARG_OFF")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D${CMAKE_SAFETYTEST}")

if(CMAKE_SAFETYTEST STREQUAL "CMAKE_SAFETYTEST_ARG_ON")
  #安全测试选项
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -fsanitize=undefined,address -O2")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fsanitize=undefined,address -O2")
endif()

#定义宏定义给UT代码使用（工程路径、应用程序输出路径、库输出路径、UT工程路径）
ADD_DEFINITIONS(-D_SOURCEDIR=\"${CMAKE_SOURCE_DIR}\")
ADD_DEFINITIONS(-D_BINARYDIR=\"${CMAKE_BINARY_DIR}\")
ADD_DEFINITIONS(-D_LIBRARYDIR=\"${LIBRARY_OUTPUT_PATH}\")
ADD_DEFINITIONS(-D_UTSOURCEDIR=\"${CMAKE_CURRENT_SOURCE_DIR}\")

#查找依赖关系的包信息
find_package(GTest REQUIRED)

#指定头文件路径
include_directories(include) #提供stub.h、addr_pri.h
include_directories(${GTEST_INCLUDE_DIRS})
include_directories(${HFILES_OUTPUT_PATH})

add_subdirectory(src)

add_subdirectory(3rdparty)

