# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

#1.定义cmake版本
cmake_minimum_required(VERSION 3.9.5)

#2.设置项目名称
set(EXE_NAME libpigzplugin)
project(${EXE_NAME}_test)

#查找依赖关系的包信息
find_package(PkgConfig REQUIRED)
find_package(Qt5Widgets REQUIRED)
include(FindPkgConfig)


pkg_search_module(DtkWidget REQUIRED dtkwidget)
include_directories(${DtkWidget_INCLUDE_DIRS})

#指定头文件路径
include_directories(${DtkWidget_INCLUDE_DIRS})
include_directories(${PROJECT_SOURCE_DIR})
include_directories(${HFILES_OUTPUT_PATH})
include_directories(${PROJECT_SOURCE_DIR}/${3RDPARTYRELATIVEPATH}/libpigzplugin)

#链接库路径
link_directories(${LIBRARY_OUTPUT_PATH})

#源文件、头文件和资源文件
file(GLOB_RECURSE c_files
    ${3RDPARTYRELATIVEPATH}/libpigzplugin/*.cpp
    ./*.cpp
)

file(GLOB_RECURSE h_files
    ${3RDPARTYRELATIVEPATH}/libpigzplugin/*.h
    ./*.h
)

file(GLOB_RECURSE json_files
    ${3RDPARTYRELATIVEPATH}/libpigzplugin/*.json
)

#生成可执行文件
add_executable(${EXE_NAME}_test ${h_files} ${c_files} ${json_files})

#链接动静态库
target_link_libraries(${EXE_NAME}_test
    Qt5::Widgets
    compressor-interface
    ${GTEST_LIBRARIES}
    ${GTEST_MAIN_LIBRARIES}
    pthread
)
