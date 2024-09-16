# SPDX-FileCopyrightText: None
#
# SPDX-License-Identifier: CC0-1.0

cmake_minimum_required(VERSION 3.11)

project(dde-appearance)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX /usr)
endif ()
include(GNUInstallDirs)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Werror=return-type -fno-omit-frame-pointer -Wextra")
endif()

add_subdirectory(src)
add_subdirectory(misc)
add_subdirectory(fakewm)
