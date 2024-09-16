# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

PROJECT(interface)
CMAKE_MINIMUM_REQUIRED(VERSION 3.10)

set(REQUIRED_QT_VERSION 5.11.3)
find_package(Qt5 ${REQUIRED_QT_VERSION} REQUIRED COMPONENTS Core Gui Widgets)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(AUTOMOC_COMPILER_PREDEFINES ON)

if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX /usr)
endif ()

file(GLOB INTERFACES
    widgetsglobal.h
    widgetsinterface.h)

set(HEADERS
    ${INTERFACES}
    widgetsinterface_p.h
)
set(SOURCES
    widgetsinterface.cpp
)

add_library(${PROJECT_NAME} SHARED ${HEADERS} ${SOURCES})
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES OUTPUT_NAME dde-widgets)

set(COMMON_LIBS
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
)

target_link_libraries(${PROJECT_NAME} PUBLIC ${COMMON_LIBS})

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}
)

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_LIBDIR})

set(INCLUDE_INSTALL_DIR include/dde-widgets)
set(PLUGIN_INSTALL_DIR ${CMAKE_INSTALL_FULL_LIBDIR}/dde-widgets/plugins)
install(FILES ${INTERFACES} DESTINATION include/dde-widgets)

include(CMakePackageConfigHelpers)
configure_package_config_file(DdeWidgetsConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/DdeWidgetsConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/DdeWidgets
    PATH_VARS INCLUDE_INSTALL_DIR PLUGIN_INSTALL_DIR
    INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/DdeWidgetsConfig.cmake DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/DdeWidgets)
