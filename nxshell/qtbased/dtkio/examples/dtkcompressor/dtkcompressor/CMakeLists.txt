# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later
# 1.cmake verson
cmake_minimum_required(VERSION 3.13)
#set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
# 2.project name
PROJECT(testcompressor)


find_package(Qt${QT_VERSION_MAJOR}Widgets)
find_package(PkgConfig REQUIRED)
find_package(DtkCore REQUIRED)
find_package(KF5Codecs REQUIRED)

# 4.source directory
AUX_SOURCE_DIRECTORY(. DIR_SRCS)

# 5.set environment variable
SET(SRC_PATH ${DIR_SRCS})

# 6.add executable file
file(GLOB_RECURSE INCLUDE_FILES "${PROJECT_SOURCE_DIR}/include/dtkcompressor/*")
ADD_EXECUTABLE(${PROJECT_NAME} ${SRC_PATH} ${INCLUDE_FILES})

include_directories(../include)
target_include_directories(${PROJECT_NAME} PUBLIC
#    ${PROJECT_SOURCE_DIR}
    Qt${QT_VERSION_MAJOR}::DBus)

set(TARGET_LIBS
    Qt${QT_VERSION_MAJOR}::Widgets
    ${DtkCore_LIBRARIES}
    KF5::Codecs
    Qt${QT_VERSION_MAJOR}::DBus
    dtkcompressor
    )

# 7.add link library
TARGET_LINK_LIBRARIES(${PROJECT_NAME}  ${TARGET_LIBS})


