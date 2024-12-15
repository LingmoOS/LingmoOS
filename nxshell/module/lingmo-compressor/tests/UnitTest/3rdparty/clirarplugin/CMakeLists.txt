# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

#1.定义cmake版本
cmake_minimum_required(VERSION 3.9.5)

#2.设置项目名称
set(EXE_NAME clirarplugin)
project(${EXE_NAME}_test)

#设置项目名称
find_package(PkgConfig REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(KF5Codecs REQUIRED)
include(FindPkgConfig)


#指定头文件路径
include_directories(${PROJECT_SOURCE_DIR})
include_directories(${HFILES_OUTPUT_PATH})
include_directories(${PROJECT_SOURCE_DIR}/${3RDPARTYRELATIVEPATH}/clirarplugin)

#源文件、头文件和资源文件
file(GLOB_RECURSE c_files
    ${3RDPARTYRELATIVEPATH}/clirarplugin/*.cpp
    ./*.cpp
)

file(GLOB_RECURSE h_files
    ${3RDPARTYRELATIVEPATH}/clirarplugin/*.h
    ./*.h
)

file(GLOB_RECURSE json_files
    ${3RDPARTYRELATIVEPATH}/clirarplugin/*.json
)

#生成可执行文件
add_executable(${EXE_NAME}_test ${h_files} ${c_files} ${json_files})

target_link_libraries(${EXE_NAME}_test
    Qt5::Widgets
    KF5::Codecs
    compressor-interface
    ${GTEST_LIBRARIES}
    ${GTEST_MAIN_LIBRARIES}
    pthread
)
