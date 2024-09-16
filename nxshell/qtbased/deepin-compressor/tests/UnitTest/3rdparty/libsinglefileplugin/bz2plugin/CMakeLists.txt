# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

#定义cmake版本
cmake_minimum_required(VERSION 3.9.5)

#设置项目名称
set(EXE_NAME bz2plugin)
project(${EXE_NAME}_test)

#查找依赖关系的包信息
find_package(PkgConfig REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(KF5Codecs REQUIRED)
find_package(KF5Archive)
include(FindPkgConfig)

pkg_search_module(DtkWidget REQUIRED dtkwidget)

#指定头文件路径
include_directories(${DtkWidget_INCLUDE_DIRS})
include_directories(${PROJECT_SOURCE_DIR})
include_directories(${HFILES_OUTPUT_PATH})
include_directories(${PROJECT_SOURCE_DIR}/../${3RDPARTYRELATIVEPATH}/libsinglefileplugin/bz2plugin)
include_directories(${PROJECT_SOURCE_DIR}/../${3RDPARTYRELATIVEPATH}/libsinglefileplugin/singlefile)

#链接库路径
link_directories(${LIBRARY_OUTPUT_PATH})

#源文件、头文件和资源文件
file(GLOB_RECURSE c_files
    ../${3RDPARTYRELATIVEPATH}/libsinglefileplugin/bz2plugin/*.cpp
    ../${3RDPARTYRELATIVEPATH}/libsinglefileplugin/singlefile/*.cpp
    ./*.cpp
)

file(GLOB_RECURSE h_files
    ../${3RDPARTYRELATIVEPATH}/libsinglefileplugin/bz2plugin/*.h
    ../${3RDPARTYRELATIVEPATH}/libsinglefileplugin/singlefile/*.h
    ./*.h
)

file(GLOB_RECURSE json_files
    ../${3RDPARTYRELATIVEPATH}/libsinglefileplugin/bz2plugin/*.json
)

#生成动静态库
add_executable(${EXE_NAME}_test ${h_files} ${c_files} ${json_files})
target_link_libraries(${EXE_NAME}_test
    Qt5::Widgets
    KF5::Codecs
    KF5::Archive
#    compressor-singlefile
compressor-interface
    ${GTEST_LIBRARIES}
    ${GTEST_MAIN_LIBRARIES}
    pthread
)
