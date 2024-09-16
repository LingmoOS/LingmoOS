# SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later

set(LIB_NAME ${CMAKE_PROJECT_NAME})

file(GLOB_RECURSE PUBLIC_HEADER "../include/*.h")
file(GLOB_RECURSE SRCS
    "./*.h"
    "./*.cpp"
)

add_library(${LIB_NAME} SHARED
    ${PUBLIC_HEADER}
    ${SRCS}
)

target_compile_definitions(${LIB_NAME} PRIVATE VERSION="${CMAKE_PROJECT_VERSION}")

target_include_directories(${LIB_NAME}
PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/dbus>
INTERFACE
    $<INSTALL_INTERFACE:${INCLUDE_INSTALL_DIR}>
)

target_link_directories(${LIB_NAME} INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    $<INSTALL_INTERFACE:${LIBRARY_INSTALL_DIR}>
)

target_link_libraries(${LIB_NAME}
PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::DBus
    Dtk::Core
PRIVATE
    PkgConfig::RSVG2
)

set_target_properties(${LIB_NAME} PROPERTIES
    VERSION ${CMAKE_PROJECT_VERSION}
    SOVERSION ${CMAKE_PROJECT_VERSION_MAJOR}
    EXPORT_NAME Template
    PUBLIC_HEADER "${PUBLIC_HEADER}"
)

install(TARGETS ${LIB_NAME} EXPORT DtkTemplateTargets DESTINATION ${LIBRARY_INSTALL_DIR} PUBLIC_HEADER DESTINATION ${INCLUDE_INSTALL_DIR})

install(EXPORT DtkTemplateTargets
    FILE DtkTemplateTargets.cmake
    NAMESPACE Dtk::
    DESTINATION ${ConfigPackageLocation}
)
