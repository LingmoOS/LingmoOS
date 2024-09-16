# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

PROJECT(dde-calendarwidget-plugin)
CMAKE_MINIMUM_REQUIRED(VERSION 3.10)

set(REQUIRED_QT_VERSION 5.11.3)
find_package(Qt5 ${REQUIRED_QT_VERSION} REQUIRED COMPONENTS Core Gui Widgets DBus)
find_package(DtkCore REQUIRED)
find_package(DtkGui REQUIRED)
find_package(DtkWidget REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(AUTOMOC_COMPILER_PREDEFINES ON)

SET(LIBRARY_OUTPUT_PATH ../app/plugins)
add_library(${PROJECT_NAME} SHARED plugin.h plugin.cpp)

set(COMMON_LIBS
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::DBus
    ${DtkWidget_LIBRARIES}
)

target_link_libraries(${PROJECT_NAME} PUBLIC ${COMMON_LIBS})
target_link_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/../interface)
target_link_libraries(${PROJECT_NAME} PRIVATE dde-widgets)

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}
    ${DtkWidget_INCLUDE_DIRS}
)
include_directories(../interface/)

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_LIBDIR}/dde-widgets/plugins)
