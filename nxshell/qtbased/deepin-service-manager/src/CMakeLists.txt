# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(SRC_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    ADD_DEFINITIONS(-DSERVICE_CONFIG_DIR="${CMAKE_BINARY_DIR}/share/")
    ADD_DEFINITIONS(-DSERVICE_LIB_DIR="${CMAKE_BINARY_DIR}/lib/")
else()
    ADD_DEFINITIONS(-DSERVICE_CONFIG_DIR="${CMAKE_INSTALL_PREFIX}/share/deepin-service-manager/")
    ADD_DEFINITIONS(-DSERVICE_LIB_DIR="${CMAKE_INSTALL_FULL_LIBDIR}/deepin-service-manager/")
endif()

add_subdirectory("core")
add_subdirectory("deepin-service-manager")
add_subdirectory("libqdbusservice")

add_subdirectory("plugin-qt")
add_subdirectory("plugin-sdbus")

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_subdirectory("demo")
endif()
