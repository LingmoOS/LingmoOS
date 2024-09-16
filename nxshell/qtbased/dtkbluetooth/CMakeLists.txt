# SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later

cmake_minimum_required(VERSION 3.13)

# If do't define version number, specify the version number
set(DTK_VERSION "5.6.12" CACHE STRING "define project version")

project(DtkBluetooth
    LANGUAGES CXX
    HOMEPAGE_URL https://github.com/linuxdeepin/dtkbluetooth
    DESCRIPTION "BlueZ DBus interface binding for Qt"
    VERSION ${DTK_VERSION}
)

if(${PROJECT_VERSION_MAJOR} STREQUAL "5")
    set(QT_VERSION_MAJOR "5")
elseif(${PROJECT_VERSION_MAJOR} STREQUAL "6")
    set(QT_VERSION_MAJOR "6")
    set(DTK_VERSION_MAJOR "6")
endif()

# Install settings
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX /usr)
endif()

set(LIB_NAME dtk${DTK_VERSION_MAJOR}bluetooth)
set(DtkBluetooth Dtk${DTK_VERSION_MAJOR}Bluetooth)

# specify install dir
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED on)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -Wall -Wextra")
set(CMAKE_SHARED_LINKER_FLAGS ${CMAKE_SHARED_LINKER_FLAGS} "-Wl,--as-needed")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(INCLUDE_INSTALL_DIR ${CMAKE_INSTALL_INCLUDEDIR}/dtk${DTK_VERSION_MAJOR}/DBluetooth)
set(LIBRARY_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR} CACHE STRING "Install dir for cmake config files")
set(CONFIG_CMAKE_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/${DtkBluetooth}" CACHE STRING "Install dir for cmake config files")

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose Debug or Release" FORCE)
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(BUILD_TESTING ON)
    set(BUILD_EXAMPLES ON)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
endif ()

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core DBus Bluetooth)
find_package(Dtk${DTK_VERSION_MAJOR} REQUIRED COMPONENTS Core)

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

# examples
if (BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

target_link_directories(${LIB_NAME} INTERFACE
    $<INSTALL_INTERFACE:${LIBRARY_INSATLL_DIR}>
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/${DtkBluetooth}ConfigVersion.cmake
    VERSION ${DTK_VERSION}
    COMPATIBILITY SameMajorVersion
)

configure_package_config_file(misc/DtkBluetoothConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/${DtkBluetooth}Config.cmake
    INSTALL_DESTINATION ${CONFIG_CMAKE_INSTALL_DIR}
    PATH_VARS INCLUDE_INSTALL_DIR LIBRARY_INSTALL_DIR
)

# for pc file config
set(PC_LIBS_PRIVATE Qt${QT_VERSION_MAJOR}Core)
set(PC_REQ_PUBLIC "dtkcore")

# config pkgconfig file
set(PC_REQUIRES "Qt${QT_VERSION_MAJOR}Core Qt${QT_VERSION_MAJOR}DBus Qt${QT_VERSION_MAJOR}Bluetooth dtkcore")
configure_file(misc/${LIB_NAME}.pc.in ${LIB_NAME}.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${LIB_NAME}.pc DESTINATION ${LIBRARY_INSTALL_DIR}/pkgconfig)
# config cmake file
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${DtkBluetooth}Config.cmake ${CMAKE_CURRENT_BINARY_DIR}/${DtkBluetooth}ConfigVersion.cmake DESTINATION ${CONFIG_CMAKE_INSTALL_DIR})

# config qmake moudule file
set(DTK_MODULE ${LIB_NAME})
set(DTK_DEPS "core dbus dtkcore")
set(DTK_CFG "v2 ltcg")
set(QMKSPECS_INSTALL_DIR "${LIBRARY_INSTALL_DIR}/qt${QT_VERSION_MAJOR}/mkspecs/modules" CACHE STRING "INSTALL DIR FOR qt pri files")
configure_file(misc/qt_lib_${LIB_NAME}.pri.in qt_lib_${LIB_NAME}.pri @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/qt_lib_${LIB_NAME}.pri DESTINATION "${QMKSPECS_INSTALL_DIR}")
