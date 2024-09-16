# SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: CC0-1.0

CMAKE_MINIMUM_REQUIRED(VERSION 3.16)

set(DTL_VERSION "0.0.1" CACHE STRING "Define project version")
set(DOCK_VERSION "6.0.37" CACHE STRING "Dock compatible version")

project(dde-tray-loader
    VERSION "${DTL_VERSION}"
    DESCRIPTION "dde-tray-loader"
    HOMEPAGE_URL "https://github.com/linuxdeepin/dde-tray-loader"
    LANGUAGES CXX C
)

if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX /usr)
endif ()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(DTL_BUILD_WITH_QT6 ON CACHE BOOL "Build dde-tray-loader with Qt6")

if (DTL_BUILD_WITH_QT6)
    set(QT_VERSION_MAJOR 6)
    set(DTK_VERSION_MAJOR 6)
else()
    set(QT_VERSION_MAJOR 5)
    set(DTK_VERSION_MAJOR "")
endif()

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

add_subdirectory(plugins)
add_subdirectory(src)

configure_file(dde-dock.pc.in dde-dock.pc @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/dde-dock.pc
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

## dev files
file(GLOB INTERFACES "interfaces/*.h"
                     "src/tray-wayland-intergration/plugin.h")
install(FILES ${INTERFACES}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/dde-dock)

## protocol
file(GLOB PROTOCOL "src/protocol/*.xml")
install(FILES ${PROTOCOL}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/dde-tray-loader/protocol)

configure_file(dde-tray-loader.pc.in ${CMAKE_BINARY_DIR}/dde-tray-loader.pc @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/dde-tray-loader.pc
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

configure_file(
    ${CMAKE_SOURCE_DIR}/cmake/DdeTrayLoader/DdeTrayLoaderConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/DdeTrayLoaderConfig.cmake
    @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/DdeTrayLoaderConfig.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/DdeTrayLoader)

configure_file(
    ${CMAKE_SOURCE_DIR}/cmake/DdeDock/DdeDockConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/DdeDockConfig.cmake
    @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/DdeDockConfig.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/DdeDock)

configure_file(environments.h.in environments.h @ONLY)
