# SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

cmake_minimum_required(VERSION 3.18)

set(PLUGIN_NAME dock)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS DBus Network)
find_package(DdeControlCenter REQUIRED)
find_package(Dtk6 COMPONENTS Widget REQUIRED)

file(GLOB_RECURSE Dock_Plugin_SRCS
    "operation/*.cpp"
    "operation/*.hpp"
    "operation/*.h"
    "res/oceanui-dock-plugin.qrc" 
)

add_library(${PLUGIN_NAME} MODULE
    ${Dock_Plugin_SRCS}
)

target_include_directories(${PLUGIN_NAME} PRIVATE
    Dde::Control-Center
    src/oceanui-dock-plugin/operation
)

target_link_libraries(${PLUGIN_NAME} PRIVATE
    Dtk6::Widget
    Qt6::Gui
    Qt6::DBus
    Dde::Control-Center
)

oceanui_install_plugin(NAME ${PLUGIN_NAME} TARGET ${PLUGIN_NAME})
oceanui_handle_plugin_translation(NAME ${PLUGIN_NAME})
