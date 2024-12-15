# SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later

cmake_minimum_required(VERSION 3.16)

project(wayland_test)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(AUTOMOC_COMPILER_PREDEFINES ON)

find_package(Qt${QT_VERSION_MAJOR} ${REQUIRED_QT_VERSION} REQUIRED COMPONENTS Core Gui WaylandClient)
if(${QT_VERSION_MAJOR} STREQUAL "5")
    find_package(Qt5 REQUIRED COMPONENTS XkbCommonSupport)
endif()

find_package(PkgConfig REQUIRED)
pkg_check_modules(WAYLAND_CLIENT REQUIRED wayland-client)

find_package(TreelandProtocols)
set(TreelandProtocols_FOUND ${TreelandProtocols_FOUND})

add_executable(${PROJECT_NAME} main.cpp)

if("${QT_VERSION_MAJOR}" STREQUAL "6")
    qt6_generate_wayland_protocol_client_sources(${PROJECT_NAME} FILES
        ${TREELAND_PROTOCOLS_DATA_DIR}/treeland-personalization-manager-v1.xml
    )
else()
    # ECM setup
    include(FeatureSummary)
    find_package(ECM REQUIRED NO_MODULE)
    set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${ECM_MODULE_PATH}")
    find_package(QtWaylandScanner)
    ecm_add_qtwayland_client_protocol(TABLET_SRCS
        PROTOCOL ${TREELAND_PROTOCOLS_DATA_DIR}/treeland-personalization-manager-v1.xml
        BASENAME treeland-personalization-manager-v1
    )
    target_sources(${PROJECT_NAME} PRIVATE ${TABLET_SRCS})

    target_link_libraries(${PROJECT_NAME} PRIVATE
        Qt${QT_VERSION_MAJOR}::XkbCommonSupportPrivate
    )
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::GuiPrivate
    Qt${QT_VERSION_MAJOR}::CorePrivate
    Qt${QT_VERSION_MAJOR}::WaylandClientPrivate
    ${WAYLAND_CLIENT_LIBRARIES}
)
