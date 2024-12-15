# SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: CC0-1.0

set(PLUGIN_NAME "appsearch")

project(${PLUGIN_NAME})

# Sources files
file(GLOB SRCS "../../interfaces/*.*")

find_package(Qt5 ${QT_MINIMUM_VERSION} REQUIRED
    COMPONENTS DBus Concurrent
)

add_definitions("${QT_DEFINITIONS} -DQT_PLUGIN")
add_library(${PLUGIN_NAME} SHARED ${SRCS})
target_sources(${PLUGIN_NAME}
PRIVATE
    appsearchplugin.cpp
    appsearchplugin.h
)
set_target_properties(${PLUGIN_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ../)

target_include_directories(${PLUGIN_NAME} PUBLIC
    ../../interfaces/
)

target_link_libraries(${PLUGIN_NAME} PRIVATE
  Qt5::DBus
  Qt5::Concurrent
)
