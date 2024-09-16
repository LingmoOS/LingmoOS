# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later

cmake_minimum_required(VERSION 3.13)

set(DTK_VERSION
    "5.6.8"
    CACHE STRING "define project version"
)

project(qt5platform-plugins
    VERSION ${DTK_VERSION}
    DESCRIPTION "DTK platform plugin module"
    HOMEPAGE_URL "https://github.com/linuxdeepin/qt5platform-plugins"
    LANGUAGES CXX C
)

if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX /usr)
endif ()

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

option(BUILD_TESTING "Build tests" ON)

if("${PROJECT_VERSION_MAJOR}" STREQUAL "5")
    set(QT_VERSION_MAJOR "5")
elseif("${PROJECT_VERSION_MAJOR}" STREQUAL "6")
    set(QT_VERSION_MAJOR "6")
    set(DTK_VERSION_MAJOR "6")
else()
    message(SEND_ERROR "not support Prject Version ${PROJECT_VERSION}.")
endif()
message(${PROJECT_VERSION_MAJOR})

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(REQUIRED_QT_VERSION 5.15.0)
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(BUILD_TESTING ON)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g -fno-omit-frame-pointer")
else ()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Ofast")
endif ()
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/plugins/platforms)

add_subdirectory(xcb)
if("${PROJECT_VERSION_MAJOR}" STREQUAL "5")
    add_subdirectory(wayland)
endif()
if(BUILD_TESTING AND NOT ${CMAKE_SYSTEM_PROCESSOR} MATCHES "loongarch")
    enable_testing()
    add_subdirectory(tests)
endif()
message(${PROJECT_VERSION_MAJOR})
