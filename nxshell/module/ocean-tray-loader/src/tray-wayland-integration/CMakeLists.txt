# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: CC0-1.0

find_package(Qt${QT_VERSION_MAJOR} ${REQUIRED_QT_VERSION} REQUIRED COMPONENTS Core Gui WaylandClient)
find_package(Dtk${DTK_VERSION_MAJOR} REQUIRED COMPONENTS Gui Widget)

find_package(ECM REQUIRED MO_MODULE)
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${ECM_MODULE_PATH}")
find_package(PkgConfig REQUIRED)

include(KDEInstallDirs)

add_library(dockpluginmanager-interface SHARED
    plugin.h
    plugin.cpp
)

target_include_directories(dockpluginmanager-interface PUBLIC
    "interfaces/"
)

target_link_libraries(dockpluginmanager-interface
PUBLIC
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
    Dtk${DTK_VERSION_MAJOR}::Widget
)

add_library(dockpluginmanager SHARED
    pluginmanagerintegration_p.h
    pluginmanagerintegration.cpp
    pluginsurface_p.h
    pluginsurface.cpp
    main.cpp
)

qt_generate_wayland_protocol_client_sources(dockpluginmanager
    FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/../protocol/plugin-manager-v1.xml
)

target_link_libraries(dockpluginmanager
PUBLIC
    dockpluginmanager-interface
PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::GuiPrivate
    Qt${QT_VERSION_MAJOR}::WaylandClient
    Qt${QT_VERSION_MAJOR}::WaylandClientPrivate
    Wayland::Client
)

set_target_properties(dockpluginmanager PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/plugins/wayland-shell-integration"
    OUTPUT_NAME plugin-shell
)

set_target_properties(dockpluginmanager-interface PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/plugins/"
    OUTPUT_NAME ocean-trayplugin-interface
    VERSION ${CMAKE_PROJECT_VERSION}
    SOVERSION ${CMAKE_PROJECT_VERSION_MAJOR}
)

# Do not install .so development library for we don't want others to link to it.
install(TARGETS dockpluginmanager-interface DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(TARGETS dockpluginmanager DESTINATION ${KDE_INSTALL_QTPLUGINDIR}/wayland-shell-integration)
