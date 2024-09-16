# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: CC0-1.0

CMAKE_MINIMUM_REQUIRED(VERSION 3.10)

set(DS_VERSION "0.0.34" CACHE STRING "Define project version")
project(DDEShell
    VERSION "${DS_VERSION}"
    DESCRIPTION "dde-shell"
    HOMEPAGE_URL "https://github.com/linuxdeepin/dde-shell"
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
option(BUILD_WITH_X11 "Build X11 emulation" ON)

set(DS_BUILD_WITH_QT6 ON CACHE BOOL "Build dde-shell with Qt6")

if (DS_BUILD_WITH_QT6)
    set(QT_VERSION_MAJOR 6)
    set(DTK_VERSION_MAJOR 6)
else()
    set(QT_VERSION_MAJOR 5)
    set(DTK_VERSION_MAJOR "")
endif()

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(LIB_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}" CACHE STRING "Library install path")
set(DDE_SHELL_PACKAGE_INSTALL_DIR ${CMAKE_INSTALL_DATADIR}/dde-shell CACHE STRING "Package install path")
set(DDE_SHELL_PLUGIN_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR}/dde-shell CACHE STRING "Plugin install path")
set(INCLUDE_INSTALL_DIR "${CMAKE_INSTALL_INCLUDEDIR}/dde-shell" CACHE STRING "Headers install path")
set(CONFIG_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/DDEShell" CACHE STRING "CMake config file install directory")
set(QML_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/qt${QT_VERSION_MAJOR}/qml" CACHE STRING "Qml plugin install directory")
set(DDE_SHELL_TRANSLATION_INSTALL_DIR "${CMAKE_INSTALL_DATADIR}/dde-shell" CACHE STRING "Translation install directory")

find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake;${CMAKE_MODULE_PATH};${ECM_MODULE_PATH};${PROJECT_SOURCE_DIR}/cmake")
include(DDEShellPackageMacros)

find_package(Qt${QT_VERSION_MAJOR} ${REQUIRED_QT_VERSION} REQUIRED COMPONENTS Core Gui Concurrent Quick WaylandClient DBus LinguistTools)
find_package(Dtk${DTK_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui)
find_package(WaylandProtocols REQUIRED)
find_package(PkgConfig REQUIRED)

add_subdirectory(frame)
add_subdirectory(shell)
add_subdirectory(example)
add_subdirectory(panels)
add_subdirectory(applets)

configure_package_config_file(
    "${CMAKE_CURRENT_LIST_DIR}/misc/DDEShellConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/DDEShellConfig.cmake"
    INSTALL_DESTINATION "${CONFIG_INSTALL_DIR}"
)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/DDEShellConfigVersion.cmake"
    VERSION ${VERSION}
    COMPATIBILITY SameMajorVersion
)
# Install cmake config file
install(FILES "${PROJECT_SOURCE_DIR}/cmake/DDEShellPackageMacros.cmake" DESTINATION "${CONFIG_INSTALL_DIR}")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/DDEShellConfig.cmake" DESTINATION "${CONFIG_INSTALL_DIR}")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/DDEShellConfigVersion.cmake" DESTINATION "${CONFIG_INSTALL_DIR}")
