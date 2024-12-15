# SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: CC0-1.0

CMAKE_MINIMUM_REQUIRED(VERSION 3.16)

cmake_policy(SET CMP0160 OLD)
set(DTL_VERSION "0.0.1" CACHE STRING "Define project version")
set(DOCK_VERSION "6.0.37" CACHE STRING "Dock compatible version")

project(ocean-tray-loader
    VERSION "${DTL_VERSION}"
    DESCRIPTION "ocean-tray-loader"
    HOMEPAGE_URL "https://github.com/lingmoos/ocean-tray-loader"
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
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-undefined")

set(DTL_BUILD_WITH_QT6 ON CACHE BOOL "Build ocean-tray-loader with Qt6")

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

configure_file(ocean-dock.pc.in ocean-dock.pc @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/ocean-dock.pc
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

## dev files
file(GLOB INTERFACES "interfaces/*.h")
install(FILES ${INTERFACES}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ocean-dock)

## protocol
file(GLOB PROTOCOL "src/protocol/*.xml")
install(FILES ${PROTOCOL}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ocean-tray-loader/protocol)

configure_file(ocean-tray-loader.pc.in ${CMAKE_BINARY_DIR}/ocean-tray-loader.pc @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/ocean-tray-loader.pc
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
