# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-undefined")

configure_file(environments.h.in environments.h @ONLY)

find_package(PkgConfig REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} ${REQUIRED_QT_VERSION} COMPONENTS Core DBus Gui Qml WaylandCompositor Widgets WaylandClient)
find_package(DdeTrayLoader REQUIRED)
find_package(TreelandProtocols REQUIRED)
pkg_check_modules(WaylandClient REQUIRED IMPORTED_TARGET wayland-client)

file(
    GLOB dock_panel_sources
    constants.h
    dockabstractsettingsconfig.h
    dockdbusproxy.cpp
    dockdbusproxy.h
    dockiteminfo.cpp
    dockiteminfo.h
    dockpanel.cpp
    dockpanel.h
    docksettings.cpp
    docksettings.h
    dockhelper.h
    dockhelper.cpp
    waylanddockhelper.h
    waylanddockhelper.cpp
    loadtrayplugins.h
    loadtrayplugins.cpp
)

# Old dbus interface compatible
qt_add_dbus_adaptor(dock_panel_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/api/old/org.lingmo.ocean.dock1.xml
    dockdbusproxy.h
    dock::DockDBusProxy
    dockfrontadaptor
    DockFrontAdaptor
)
qt_add_dbus_adaptor(dock_panel_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/api/old/org.lingmo.ocean.daemon.Dock1.xml
    dockdbusproxy.h
    dock::DockDBusProxy
    dockdaemonadaptor
    DockDaemonAdaptor
)

# dock dbus api
qt_add_dbus_adaptor(dock_panel_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/api/dbus/org.lingmo.ds.dock.xml
    dockpanel.h
    dock::DockPanel
    dockadaptor
    DockAdaptor
)

add_library(dockpanel SHARED
    ${dock_panel_sources}
)

qt_generate_wayland_protocol_client_sources(dockpanel
    FILES
        ${TREELAND_PROTOCOLS_DATA_DIR}/treeland-wallpaper-color-v1.xml
        ${TREELAND_PROTOCOLS_DATA_DIR}/treeland-ocean-shell-v1.xml
)

target_link_libraries(dockpanel PRIVATE
    PkgConfig::WaylandClient
    Qt${QT_VERSION_MAJOR}::WaylandClient
    Qt${QT_VERSION_MAJOR}::WaylandClientPrivate
    Qt${QT_VERSION_MAJOR}::Widgets
    ocean-shell-frame
)

if (BUILD_WITH_X11)
    target_compile_definitions(dockpanel PRIVATE BUILD_WITH_X11=)
    pkg_check_modules(XCB REQUIRED IMPORTED_TARGET xcb xcb-aux xcb-res xcb-ewmh)
    target_sources(dockpanel PRIVATE
        x11dockhelper.h
        x11dockhelper.cpp
    )

    target_link_libraries(dockpanel PUBLIC
        PkgConfig::XCB
    )
endif(BUILD_WITH_X11)

ds_install_package(PACKAGE org.lingmo.ds.dock TARGET dockpanel)
ds_handle_package_translation(PACKAGE org.lingmo.ds.dock)

# sub plugins
add_subdirectory(showdesktop)
add_subdirectory(taskmanager)
add_subdirectory(tray)
add_subdirectory(multitaskview)

find_package(DdeControlCenter)

if (DdeControlCenter_FOUND)
    add_subdirectory(oceanui-dock-plugin)
endif()

add_subdirectory(searchitem)
#add_subdirectory(appruntimeitem)

# dock qml element(include Dock.xx defines and DockCompositor)
file(
    GLOB plugin_sources
    constants.h
    # dockfilterproxymodel.cpp
    # dockfilterproxymodel.h
    pluginmanagerextension_p.h
    pluginmanagerextension.cpp
    pluginmanagerintegration_p.h
    pluginmanagerintegration.cpp
    dockpositioner.h
    dockpositioner.cpp
)

set_source_files_properties(DockCompositor.qml PROPERTIES
    QT_QML_SINGLETON_TYPE TRUE
)

set_source_files_properties(MenuHelper.qml PROPERTIES
    QT_QML_SINGLETON_TYPE TRUE
)

set_source_files_properties(DockPalette.qml PROPERTIES
    QT_QML_SINGLETON_TYPE TRUE
)

qt_policy(SET QTP0001 OLD)
qt_add_qml_module(dock-plugin
    PLUGIN_TARGET dock-plugin
    URI "org.lingmo.ds.dock"
    VERSION "1.0"
    SHARED
    SOURCES ${plugin_sources}
    QML_FILES DockCompositor.qml OverflowContainer.qml MenuHelper.qml DockPalette.qml
    AppletItemButton.qml
    AppletItemBackground.qml
    OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/plugins/org/lingmo/ds/dock/
)

qt_generate_wayland_protocol_server_sources(dock-plugin
    FILES
        ${OCEAN_TRAY_LOADER_PROTOCOL}
        ${WaylandProtocols_DATADIR}/staging/fractional-scale/fractional-scale-v1.xml
)

target_link_libraries(dock-plugin PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Qml
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::WaylandCompositor
PRIVATE
    PkgConfig::WaylandClient
    Qt${QT_VERSION_MAJOR}::WaylandCompositorPrivate
)

target_include_directories(dock-plugin
    PUBLIC
    $<TARGET_PROPERTY:ocean-shell-frame,INTERFACE_INCLUDE_DIRECTORIES>
)

install(TARGETS dock-plugin DESTINATION "${QML_INSTALL_DIR}/org/lingmo/ds/dock/")
install(DIRECTORY "${PROJECT_BINARY_DIR}/plugins/org/lingmo/ds/dock/" DESTINATION "${QML_INSTALL_DIR}/org/lingmo/ds/dock/")
dtk_add_config_meta_files(APPID org.lingmo.ds.dock FILES dconfig/org.lingmo.ds.dock.json) # compat
dtk_add_config_meta_files(APPID org.lingmo.ds.dock FILES dconfig/org.lingmo.ds.dock.tray.json) # compat
dtk_add_config_meta_files(APPID org.lingmo.ocean.shell FILES dconfig/org.lingmo.ds.dock.json)
dtk_add_config_meta_files(APPID org.lingmo.ocean.shell FILES dconfig/org.lingmo.ds.dock.tray.json)
