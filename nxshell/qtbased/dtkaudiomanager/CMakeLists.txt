# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later

cmake_minimum_required(VERSION 3.13)

# If do't define version number, specify the version number
set (DTK_VERSION "5.0.0" CACHE STRING "define project version")

set(BIN_NAME dtkaudiomanager)

project(${BIN_NAME}
    LANGUAGES CXX
    HOMEPAGE_URL https://github.com/linuxdeepin/dtkaudiomanager
    DESCRIPTION "Qt-based development library for audio manager on Deepin"
    VERSION ${DTK_VERSION}
)
if (${CMAKE_PROJECT_VERSION_MAJOR} STREQUAL "5")
    set(QT_DEFAULT_MAJOR_VERSION "5")
    set(DTK_DEFAULT_MAJOR_VERSION "")
    set(DTK_VERSION_MAJOR "")
    set(EnableDtk5 on)
    set(EnableDtk6 off)
elseif(${CMAKE_PROJECT_VERSION_MAJOR} STREQUAL "6")
    set(QT_DEFAULT_MAJOR_VERSION "6")
    set(DTK_DEFAULT_MAJOR_VERSION "6")
    set(DTK_VERSION_MAJOR "6")
    set(EnableDtk5 off)
    set(EnableDtk6 on)
else()
    message(FATAL_ERROR "Only support DTK_VERSION is 5 or 6")
endif()

# specify install dir
include(GNUInstallDirs)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED on)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -Wall -Wextra")
set(CMAKE_SHARED_LINKER_FLAGS ${CMAKE_SHARED_LINKER_FLAGS} "-Wl,--as-needed")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(DAUDIO_MANAGER_PLUGIN_DIR ${CMAKE_INSTALL_FULL_LIBDIR}/${BIN_NAME}/plugins)
add_definitions(-DDAUDIO_MANAGER_PLUGIN_DIR="${DAUDIO_MANAGER_PLUGIN_DIR}")
# Install settings
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX /usr)
endif ()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(BUILD_TESTING ON)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fsanitize=address -fno-omit-frame-pointer")
else ()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Ofast")
endif ()

find_package(QT NAMES Qt${QT_DEFAULT_MAJOR_VERSION} REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core DBus)
find_package(Dtk${DTK_VERSION_MAJOR}Core REQUIRED)

add_subdirectory(src)
add_subdirectory(dde-daemon-plugin)
add_subdirectory(examples)

set(BUILD_DOCS ON CACHE BOOL "Generate doxygen-based documentation")
find_package(Doxygen)
if (BUILD_DOCS AND DOXYGEN_FOUND)
    add_subdirectory(docs)
endif ()

# test
if (BUILD_TESTING)
   add_subdirectory(tests)
endif ()

# for pc file config
set(PC_LIBS_PRIVATE Qt${QT_VERSION_MAJOR}Core)
set(PC_REQ_PRIVATE)
set(PC_REQ_PUBLIC)

# config pkgconfig file
configure_file(misc/${BIN_NAME}.pc.in ${BIN_NAME}.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${BIN_NAME}.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
# config cmake file
configure_file(misc/${BIN_NAME}Config.cmake.in ${BIN_NAME}Config.cmake @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${BIN_NAME}Config.cmake DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${BIN_NAME})

# config qmake moudule file
set(DTK_MODULE ${BIN_NAME})
set(DTK_DEPS "core dbus")
set(QMKSPECS_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/qt${QT_VERSION_MAJOR}/mkspecs/modules" CACHE STRING "INSTALL DIR FOR qt pri files")
configure_file(misc/qt_lib_${BIN_NAME}.pri.in qt_lib_${BIN_NAME}.pri @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/qt_lib_${BIN_NAME}.pri DESTINATION "${QMKSPECS_INSTALL_DIR}")
