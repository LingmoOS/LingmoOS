# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: CC0-1.0

find_package(Qt5 REQUIRED COMPONENTS Gui WaylandClient XkbCommonSupport)
find_package(Dtk REQUIRED COMPONENTS Gui Widget)

find_package(ECM REQUIRED MO_MODULE)
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${ECM_MODULE_PATH}")
find_package(QtWaylandScanner)
find_package(PkgConfig REQUIRED)

pkg_check_modules(XKBCOMMON REQUIRED IMPORTED_TARGET xkbcommon)
pkg_check_modules(WaylandClient REQUIRED IMPORTED_TARGET wayland-client)

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
    Qt5::Gui
    Qt5::Widgets
    Dtk::Widget
    PkgConfig::XKBCOMMON
)

add_library(dockpluginmanager SHARED
    pluginmanager_p.h
    pluginmanager.cpp
    pluginmanagerintegration_p.h
    pluginmanagerintegration.cpp
    pluginsurface_p.h
    pluginsurface.cpp
    main.cpp
)

ecm_add_qtwayland_client_protocol(dockpluginmanager
PROTOCOL
    ${CMAKE_CURRENT_SOURCE_DIR}/../protocol/plugin-manager-v1.xml
BASENAME
    plugin-manager-v1
)

target_link_libraries(dockpluginmanager
PUBLIC
    dockpluginmanager-interface
PRIVATE
    PkgConfig::WaylandClient
    PkgConfig::XKBCOMMON
    Qt5::WaylandClientPrivate
    Qt5::XkbCommonSupportPrivate
)

set_target_properties(dockpluginmanager PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/plugins/wayland-shell-integration"
    OUTPUT_NAME plugin-shell
)

set_target_properties(dockpluginmanager-interface PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/plugins/"
    OUTPUT_NAME dde-trayplugin-interface
    VERSION ${CMAKE_PROJECT_VERSION}
    SOVERSION ${CMAKE_PROJECT_VERSION_MAJOR}
)

# Do not install .so development library for we don't want others to link to it.
install(TARGETS dockpluginmanager-interface DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(TARGETS dockpluginmanager DESTINATION ${KDE_INSTALL_QTPLUGINDIR}/wayland-shell-integration)
