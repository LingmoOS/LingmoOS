# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later

cmake_minimum_required(VERSION 3.13)

# If do't define version number, specify the version number
set(DTK_VERSION "5.0.0" CACHE STRING "define project version")

project(DtkSystemSettings
    LANGUAGES CXX
    HOMEPAGE_URL https://github.com/linuxdeepin/dtksystemsettings
    DESCRIPTION "dtk library for systemsettings."
    VERSION ${DTK_VERSION}
)

if("${PROJECT_VERSION_MAJOR}" STREQUAL "5")
    set(QT_VERSION_MAJOR "5")
    set(DTK_VERSION_MAJOR "5")
elseif("${PROJECT_VERSION_MAJOR}" STREQUAL "6")
    set(QT_VERSION_MAJOR "6")
    set(DTK_VERSION_MAJOR "6")
    set(VERSION_SUFFIX "6")
else()
    message(SEND_ERROR "not support Prject Version ${PROJECT_VERSION}.")
endif()

# Install settings
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX /usr)
endif()
include(GNUInstallDirs) # Specify install dir
include(CMakePackageConfigHelpers)
include(CMakePrintHelpers)

set(LIB_NAME dtk${VERSION_SUFFIX}systemsettings)
set(DtkSystemSettings Dtk${VERSION_SUFFIX}SystemSettings)
set(DTK_INCLUDE_INSTALL_DIR "${CMAKE_INSTALL_INCLUDEDIR}/dtk${DTK_VERSION_MAJOR}/DSystemSettings" CACHE STRING "Include directory headers")
set(MKSPECS_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/qt${QT_VERSION_MAJOR}" CACHE STRING "Install directory for qt pri files")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED on)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -Wall -Wextra")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CONFIG_CMAKE_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/${DtkSystemSettings}" CACHE STRING "Install dir for cmake config files")

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose Debug or Release" FORCE)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(BUILD_TESTING ON)
    set(BUILD_EXAMPLES ON)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
endif()

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core DBus)
find_package(Dtk${VERSION_SUFFIX} REQUIRED Core)

file(GLOB_RECURSE LIB_PUBLIC_HEADERS "${PROJECT_SOURCE_DIR}/include/*") # Get all headers which will be exported

add_subdirectory(src)

set(BUILD_DOCS ON CACHE BOOL "Generate doxygen-based documentation")
find_package(Doxygen)

if(BUILD_DOCS AND DOXYGEN_FOUND)
    add_subdirectory(docs)
endif()

# example
if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# test
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

target_link_directories(${LIB_NAME} INTERFACE
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_LIBDIR}>
)

write_basic_package_version_file(
    ${DtkSystemSettings}ConfigVersion.cmake
    VERSION ${DTK_VERSION}
    COMPATIBILITY SameMajorVersion
)

configure_package_config_file(misc/DtkSystemSettingsConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/${DtkSystemSettings}Config.cmake
    INSTALL_DESTINATION ${CONFIG_CMAKE_INSTALL_DIR}
    PATH_VARS CMAKE_INSTALL_INCLUDEDIR CMAKE_INSTALL_LIBDIR
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${DtkSystemSettings}Config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/${DtkSystemSettings}ConfigVersion.cmake
    DESTINATION ${CONFIG_CMAKE_INSTALL_DIR}
)

GNUInstallDirs_get_absolute_install_dir(DTK_FULL_INCLUDE_INSTALL_DIR DTK_INCLUDE_INSTALL_DIR INCLUDEDIR)
set(PC_REQUIRES "Qt${QT_VERSION_MAJOR}Core Qt${QT_VERSION_MAJOR}DBus dtk${VERSION_SUFFIX}core")
configure_file(misc/dtksystemsettings.pc.in dtk${VERSION_SUFFIX}systemsettings.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/dtk${VERSION_SUFFIX}systemsettings.pc DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")

set(DTK_MODULE ${LIB_NAME})
set(DTK_DEPS "core dbus dtk${VERSION_SUFFIX}core")
set(DTK_CFG "v2 ltcg")
configure_file(misc/qt_lib_dtksystemsettings.pri.in qt_lib_dtk${VERSION_SUFFIX}systemsettings.pri @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/qt_lib_dtk${VERSION_SUFFIX}systemsettings.pri DESTINATION "${MKSPECS_INSTALL_DIR}")
