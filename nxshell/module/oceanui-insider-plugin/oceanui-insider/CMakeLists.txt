# SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: CC0-1.0
cmake_minimum_required(VERSION 3.7)

set(PLUGIN_NAME "oceanui-insider-plugin")

project(${PLUGIN_NAME})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_FLAGS "-g -Wall")

# 增加安全编译参数
ADD_DEFINITIONS("-fstack-protector-strong -D_FORTITY_SOURCE=1 -z noexecstack -pie -fPIC -z lazy")

find_package(Qt6 COMPONENTS Core Gui DBus LinguistTools REQUIRED)
find_package(DdeControlCenter REQUIRED)
find_package(PkgConfig REQUIRED)


set(OceanUIInsider_Name insider)
file(GLOB_RECURSE OceanUIInsider_SRCS
    "*.cpp"
    "*.h"
)
add_library(${OceanUIInsider_Name} MODULE
    ${OceanUIInsider_SRCS}
)

target_include_directories(${OceanUIInsider_Name} PUBLIC
    Dde::Control-Center
)

set(OceanUIInsider_Libraries
    Qt6::DBus
    Qt6::Gui
    Dde::Control-Center
)

target_link_libraries(${OceanUIInsider_Name} PRIVATE
    ${OceanUIInsider_Libraries}
)

oceanui_install_plugin(NAME ${OceanUIInsider_Name} TARGET ${OceanUIInsider_Name})
oceanui_handle_plugin_translation(NAME ${OceanUIInsider_Name})
