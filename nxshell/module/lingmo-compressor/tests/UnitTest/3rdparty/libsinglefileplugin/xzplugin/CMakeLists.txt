# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

#定义cmake版本
cmake_minimum_required(VERSION 3.9.5)

#设置项目名称
set(EXE_NAME xzplugin)
project(${EXE_NAME}_test)

#查找依赖关系的包信息
find_package(PkgConfig REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(KF5Codecs REQUIRED)
#find_package(KF5Archive)
include(FindPkgConfig)

#指定头文件路径
include_directories(${PROJECT_SOURCE_DIR})
include_directories(${HFILES_OUTPUT_PATH})
include_directories(${PROJECT_SOURCE_DIR}/../${3RDPARTYRELATIVEPATH}/libsinglefileplugin/xzplugin)

#链接库路径
link_directories(${LIBRARY_OUTPUT_PATH})

#源文件、头文件和资源文件
file(GLOB_RECURSE c_files
    ../${3RDPARTYRELATIVEPATH}/libsinglefileplugin/xzplugin/*.cpp
    ./*.cpp
)

file(GLOB_RECURSE h_files
    ../${3RDPARTYRELATIVEPATH}/libsinglefileplugin/xzplugin/*.h
    ./*.h
)

file(GLOB_RECURSE json_files
    ../${3RDPARTYRELATIVEPATH}/libsinglefileplugin/xzplugin/*.json
)

#生成动静态库
add_executable(${EXE_NAME}_test ${h_files} ${c_files} ${json_files})
target_link_libraries(${EXE_NAME}_test
    Qt5::Widgets
    KF5::Codecs
#    KF5::Archive
    compressor-singlefile
    ${GTEST_LIBRARIES}
    ${GTEST_MAIN_LIBRARIES}
    pthread
)
