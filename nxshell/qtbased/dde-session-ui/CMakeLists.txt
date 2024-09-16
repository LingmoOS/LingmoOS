# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

cmake_minimum_required(VERSION 3.13)

set(PROJECT_NAME dde-session-ui)
project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall")
option(DISABLE_SYS_UPDATE "disable sys update" OFF)

# 增加安全编译参数
ADD_DEFINITIONS("-fstack-protector-strong -D_FORTITY_SOURCE=1 -z noexecstack -pie -fPIC -z lazy")

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-fsanitize=address)
    add_link_options(-fsanitize=address)
endif()

if (DEFINED ENABLE_MIEEE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mieee")
endif()

# Install settings
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX /usr)
endif ()

include(GNUInstallDirs)

if (NOT (${CMAKE_BUILD_TYPE} MATCHES "Debug"))
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Ofast")

    # generate qm
    execute_process(COMMAND bash "translate_generation.sh"
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
endif ()

set(DAEMON_LIB_PATH "lib/deepin-daemon" CACHE STRING "daemon lib path")

# 查找gmock的cmake文件
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)

# Find the library
find_package(PkgConfig REQUIRED)
find_package(GTest REQUIRED)
find_package(GMock REQUIRED)
find_package(Dtk REQUIRED COMPONENTS Core Widget)
find_package(Qt5 REQUIRED COMPONENTS Widgets DBus X11Extras Xml Concurrent Svg Sql Network Test)

pkg_check_modules(GSETTINGS REQUIRED IMPORTED_TARGET gsettings-qt)
pkg_check_modules(DdeDockInterface REQUIRED dde-dock)
pkg_check_modules(XCB_EWMH REQUIRED IMPORTED_TARGET xcb-ewmh)
pkg_check_modules(GLIB REQUIRED glib-2.0)
pkg_check_modules(GIO REQUIRED gio-unix-2.0)
pkg_check_modules(SYSTEMD REQUIRED libsystemd)

set(Test_Libraries
    -lpthread
    -lgcov
    -lgtest
    -lgmock
)

find_package(DtkTools REQUIRED)

# shared objects
add_subdirectory("common")
add_subdirectory("widgets")
add_subdirectory("global_util")
add_subdirectory("global_util/dbus")

# child projects
add_subdirectory("dde-bluetooth-dialog")
add_subdirectory("dde-hints-dialog")
add_subdirectory("dde-license-dialog")
add_subdirectory("dde-lowpower")
add_subdirectory("dde-osd")
add_subdirectory("dde-pixmix")
add_subdirectory("dde-suspend-dialog")
add_subdirectory("dde-switchtogreeter")
add_subdirectory("dde-touchscreen-dialog")
add_subdirectory("dde-warning-dialog")
add_subdirectory("dde-welcome")
add_subdirectory("dde-wm-chooser")

# FIXME: startmanager is removed, so it need to be updated
# add_subdirectory("dmemory-warning-dialog")

add_subdirectory("dnetwork-secret-dialog")

# TODO: 需要break旧版本的控制中心
add_subdirectory("reset-password-dialog")

#----------------------------install config------------------------------
## qm files
file(GLOB QM_FILES "translations/*.qm")
install(FILES ${QM_FILES} DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/translations)
