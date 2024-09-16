# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later

project(dwayland)

find_package(Qt${QT_VERSION_MAJOR} ${REQUIRED_QT_VERSION} REQUIRED COMPONENTS Core Gui WaylandClient)
if(${QT_VERSION_MAJOR} STREQUAL "5")
    find_package(Qt5 REQUIRED COMPONENTS XkbCommonSupport)
endif()

find_package(PkgConfig REQUIRED)
pkg_check_modules(WAYLAND REQUIRED wayland-client wayland-cursor IMPORTED_TARGET)
pkg_check_modules(X11 REQUIRED xkbcommon IMPORTED_TARGET)

add_definitions(-DQT5DWAYLANDPLUGIN_LIBRARY -DQT_DEPRECATED_WARNINGS)

include(${CMAKE_CURRENT_LIST_DIR}/dwayland.cmake)

add_library(${PROJECT_NAME} MODULE main.cpp ${wayland_SRC})

target_compile_definitions(${PROJECT_NAME} 
PRIVATE
    QT5DWAYLANDPLUGIN_LIBRARY
    QT_DEPRECATED_WARNINGS
)

target_include_directories(${PROJECT_NAME}
PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

if("${QT_VERSION_MAJOR}" STREQUAL "6")
    list(GET Qt6WaylandClient_PRIVATE_INCLUDE_DIRS 0 dir)
    if(EXISTS ${dir})
        target_include_directories(${PROJECT_NAME} PRIVATE ${Qt6WaylandClient_PRIVATE_INCLUDE_DIRS})
    else()
        target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/wayland/qtwayland-dev/${Qt6_VERSION})
    endif()
endif()

target_link_libraries(${PROJECT_NAME}
PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::GuiPrivate
    Qt${QT_VERSION_MAJOR}::WaylandClient
    PkgConfig::WAYLAND
    PkgConfig::X11
)
if(${QT_VERSION_MAJOR} STREQUAL "5")
    target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::XkbCommonSupportPrivate Qt5::WaylandClientPrivate)
endif()

if(NOT INSTALL_PATH)
    set(INSTALL_PATH ${CMAKE_INSTALL_LIBDIR}/qt${QT_VERSION_MAJOR}/plugins/platforms)
endif()
install(TARGETS ${PROJECT_NAME} DESTINATION ${INSTALL_PATH})
