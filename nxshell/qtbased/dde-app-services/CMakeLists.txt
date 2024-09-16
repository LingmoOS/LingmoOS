# SPDX-FileCopyrightText: 2022 Uniontech Software Technology Co.,Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-only

cmake_minimum_required(VERSION 3.7)

set(DVERSION "1.0.0" CACHE STRING "define project version")
add_definitions(-DVERSION="${DVERSION}")

set(DTK_VERSION "5" CACHE STRING "Dtk Version")

if (DTK_VERSION STREQUAL "5")
    set(QT_VERSION_MAJOR "5")
    set(EnableDtk5 on)
    set(EnableDtk6 off)
elseif(DTK_VERSION STREQUAL "6")
    set(QT_VERSION_MAJOR "6")
    set(DTK_VERSION_MAJOR "6")
    set(EnableDtk5 off)
    set(EnableDtk6 on)
else()
    message(FATAL_ERROR "Only support DTK_VERSION is 5 or 6")
endif()

set (BUILD_DOCS ON CACHE BOOL "Generate doxygen-based documentation")

project(dde-app-services)

add_subdirectory("dconfig-center")
