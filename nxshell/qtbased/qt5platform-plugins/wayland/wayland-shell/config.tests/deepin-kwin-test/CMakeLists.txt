# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later

cmake_minimum_required(VERSION 3.13)

project(deepin-kwin-test)

set(REQUIRED_QT_VERSION 5.15.0)
find_package(Qt${QT_VERSION_MAJOR} ${REQUIRED_QT_VERSION} REQUIRED COMPONENTS WaylandClient)

include(../../wayland-shell.cmake)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(AUTOMOC_COMPILER_PREDEFINES ON)

add_executable(${PROJECT_NAME} main.cpp)

if("${QT_VERSION_MAJOR}" STREQUAL "5")
    list(APPEND COMMON_LIBS Qt5::WaylandClientPrivate)
endif()
target_link_libraries(${PROJECT_NAME} PUBLIC ${COMMON_LIBS})

if(${QT_VERSION_MAJOR} STREQUAL "6")
    list(GET Qt6WaylandClient_PRIVATE_INCLUDE_DIRS 0 dir)
    if(EXISTS ${dir})
        target_include_directories(${PROJECT_NAME} PRIVATE ${Qt6WaylandClient_PRIVATE_INCLUDE_DIRS})
    else()
        target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../../../qtwayland-dev/${Qt6_VERSION})
    endif()
endif()
target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}
)
