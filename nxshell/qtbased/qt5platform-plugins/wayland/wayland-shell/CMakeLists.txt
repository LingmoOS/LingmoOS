# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later

project(kwayland-shell)

find_package(Qt${QT_VERSION_MAJOR} ${REQUIRED_QT_VERSION} REQUIRED COMPONENTS Core Gui Widgets WaylandClient)

if(${QT_VERSION_MAJOR} STREQUAL "5")
    find_package(Qt5 REQUIRED COMPONENTS XkbCommonSupport)
endif()

include(wayland-shell.cmake)

try_compile(DEEPIN_KWIN_TEST_COMPILE_RESULT ${CMAKE_CURRENT_BINARY_DIR}/deepin-kwin-test
    ${CMAKE_CURRENT_LIST_DIR}/config.tests/deepin-kwin-test deepin-kwin-test CMAKE_FLAGS -DQT_VERSION_MAJOR=${QT_VERSION_MAJOR})

try_compile(WAYLAND_TEST_COMPILE_RESULT ${CMAKE_CURRENT_BINARY_DIR}/wayland_test
    ${CMAKE_CURRENT_LIST_DIR}/config.tests/wayland_test wayland_test CMAKE_FLAGS -DQT_VERSION_MAJOR=${QT_VERSION_MAJOR})

if(WAYLAND_TEST_COMPILE_RESULT)
    add_definitions(-DUSE_DEEPIN_WAYLAND)
    set(GLOBAL_HEADERS
        dwaylandshellmanager.h
        dkeyboard.h
        ${CMAKE_SOURCE_DIR}/src/global.h
        ${CMAKE_SOURCE_DIR}/src/vtablehook.h
    )
    set(GLOBAL_SOURCES
        dwaylandshellmanager.cpp
        dkeyboard.cpp
        ${CMAKE_SOURCE_DIR}/src/global.cpp
        ${CMAKE_SOURCE_DIR}/src/vtablehook.cpp
    )
else()
    message(WARNING "QtWayland version incompatible")
endif()

add_library(${PROJECT_NAME} MODULE main.cpp ${GLOBAL_SOURCES} ${GLOBAL_HEADERS})
set_target_properties(${PROJECT_NAME} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins/wayland-shell-integration
)

target_compile_definitions(${PROJECT_NAME}
    PRIVATE
    QT5DWAYLANDPLUGIN_LIBRARY
    QT_DEPRECATED_WARNINGS
)

if(DEEPIN_KWIN_TEST_COMPILE_RESULT)
    target_compile_definitions(${PROJECT_NAME} PRIVATE D_DEEPIN_KWIN)
endif()

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/src)

list(APPEND COMMON_LIBS
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::GuiPrivate
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::WidgetsPrivate
    Qt${QT_VERSION_MAJOR}::WaylandClient
)
if(${QT_VERSION_MAJOR} STREQUAL "5")
    list(APPEND COMMON_LIBS Qt5::XkbCommonSupportPrivate Qt5::WaylandClientPrivate)
endif()

target_link_libraries(${PROJECT_NAME} ${COMMON_LIBS})

if(NOT INSTALL_PATH)
    set(INSTALL_PATH ${CMAKE_INSTALL_LIBDIR}/qt${QT_VERSION_MAJOR}/plugins/wayland-shell-integration)
endif()
install(TARGETS ${PROJECT_NAME} DESTINATION ${INSTALL_PATH})
