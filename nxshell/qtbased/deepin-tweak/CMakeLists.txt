# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.

# SPDX-License-Identifier: CC0-1.0
cmake_minimum_required(VERSION 3.13)

project(deepin-tweak VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(GNUInstallDirs)

# generate qm
execute_process(COMMAND bash "translate_generation.sh"
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

find_package(QT NAMES Qt5 REQUIRED COMPONENTS Core Quick)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Quick)
find_package(DtkDeclarative REQUIRED)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DPLUGINPATH="${CMAKE_CURRENT_SOURCE_DIR}")
endif()

set(LIB_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/deepin-tweak")

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/lib")
    add_definitions(-DLOCALLIBPATH="${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
else()
    add_definitions(-DLOCALLIBPATH="${CMAKE_INSTALL_FULL_LIBDIR}/deepin-tweak")
endif()

file(GLOB_RECURSE QRC resources.qrc)

add_subdirectory(src)
add_subdirectory(plugins)

install(FILES misc/deepin-tweak.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
install(FILES misc/deepin-tweak.svg DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps/)
