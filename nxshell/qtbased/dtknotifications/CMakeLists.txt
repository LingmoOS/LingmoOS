# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later

cmake_minimum_required(VERSION 3.13)
# If do't define version number, specify the version number
set(DTK_VERSION "1.0.0" CACHE STRING "define project version")

project(dtknotifications
    LANGUAGES CXX
    HOMEPAGE_URL https://github.com/linuxdeepin/dtknotifications
    DESCRIPTION "notification development tool kit"
    VERSION ${DTK_VERSION})

set(ENABLE_Qt6 CACHE BOOL OFF)
if (ENABLE_Qt6)
    set(DTK_VERSION_MAJOR 6)
    set(QT_VERSION_MAJOR 6)
    set(BUILD_EXAMPLES OFF CACHE BOOL "Build examples")
else()
    set(QT_VERSION_MAJOR 5)
    set(BUILD_EXAMPLES ON CACHE BOOL "Build examples")
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

# Install settings
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX /usr)
endif ()

set(CMAKE_CXX_FLAGS Release CACHE STRING "define CXXFLAGS")

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(BUILD_TESTING ON)
    set(BUILD_EXAMPLE ON)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
endif ()

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core DBus)
find_package(Dtk${DTK_VERSION_MAJOR}Core REQUIRED)

add_subdirectory(src)

set(BUILD_DOCS ON CACHE BOOL "Generate doxygen-based documentation")
find_package(Doxygen)
if (BUILD_DOCS AND DOXYGEN_FOUND)
    add_subdirectory(docs)
endif ()

# test
if (BUILD_TESTING)
   add_subdirectory(tests)
endif ()

# example
if (BUILD_EXAMPLE)
    add_subdirectory(example)
endif ()

# for pc file config
set(PC_LIBS_PRIVATE Qt${QT_VERSION_MAJOR}Core)
set(PC_REQ_PRIVATE)
set(PC_REQ_PUBLIC)

# config pkgconfig file
configure_file(misc/${CMAKE_PROJECT_NAME}.pc.in ${CMAKE_PROJECT_NAME}.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
# config cmake file
configure_file(misc/DtkNotificationsConfig.cmake.in DtkNotificationsConfig.cmake @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/DtkNotificationsConfig.cmake DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/DtkNotifications)

# config qmake moudule file
set(DTK_MODULE ${CMAKE_PROJECT_NAME})
set(DTK_DEPS "core dbus")
set(QMKSPECS_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/qt${QT_VERSION_MAJOR}/mkspecs/modules" CACHE STRING "INSTALL DIR FOR qt pri files")
configure_file(misc/qt_lib_${CMAKE_PROJECT_NAME}.pri.in qt_lib_${CMAKE_PROJECT_NAME}.pri @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/qt_lib_${CMAKE_PROJECT_NAME}.pri DESTINATION "${QMKSPECS_INSTALL_DIR}")
