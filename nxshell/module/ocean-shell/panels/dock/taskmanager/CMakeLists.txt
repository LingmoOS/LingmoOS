# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

find_package(Qt${QT_VERSION_MAJOR} ${REQUIRED_QT_VERSION} REQUIRED COMPONENTS DBus WaylandClient)
find_package(yaml-cpp REQUIRED)
find_package(TreelandProtocols REQUIRED)

set_source_files_properties(
    ${CMAKE_CURRENT_SOURCE_DIR}/api/amdbus/org.desktopspec.ApplicationManager1.Application.xml
    PROPERTIES  INCLUDE api/types/am.h
                CLASSNAME Application
)

set_source_files_properties(
    ${CMAKE_CURRENT_SOURCE_DIR}/api/amdbus/org.desktopspec.ApplicationManager1.xml
    PROPERTIES  INCLUDE api/types/am.h
                CLASSNAME ApplicationManager
)

set_source_files_properties(
    ${CMAKE_CURRENT_SOURCE_DIR}/api/amdbus/org.desktopspec.ObjectManager1.xml
    PROPERTIES  INCLUDE api/types/am.h
                CLASSNAME ObjectManager
)

qt_add_dbus_interfaces(
    DBUS_INTERFACES
    ${CMAKE_CURRENT_SOURCE_DIR}/api/amdbus/org.desktopspec.ApplicationManager1.Application.xml
    ${CMAKE_CURRENT_SOURCE_DIR}/api/amdbus/org.desktopspec.ApplicationManager1.xml
    ${CMAKE_CURRENT_SOURCE_DIR}/api/amdbus/org.desktopspec.ObjectManager1.xml
)

qt_add_dbus_adaptor(DBUS_INTERFACES
    ${CMAKE_CURRENT_SOURCE_DIR}/api/dbus/org.lingmo.ds.dock.taskmanager.item.xml
    appitem.h
    dock::AbstractItem
    itemadaptor
    ItemAdaptor
)

qt_add_dbus_adaptor(DBUS_INTERFACES
    ${CMAKE_CURRENT_SOURCE_DIR}/api/dbus/org.lingmo.ds.dock.taskmanager.xml
    taskmanager.h
    dock::TaskManager
    taskmanageradaptor
    TaskManagerAdaptor
)

add_library(dock-taskmanager SHARED ${DBUS_INTERFACES}
    abstractwindow.h
    abstractwindowmonitor.cpp
    abstractwindowmonitor.h
    abstractitem.h
    abstractitem.cpp
    appitem.cpp
    appitem.h
    rolecombinemodel.cpp
    rolecombinemodel.h
    itemmodel.cpp
    itemmodel.h
    desktopfileabstractparser.cpp
    desktopfileabstractparser.h
    desktopfileamparser.cpp
    desktopfileamparser.h
    desktopfileparserfactory.h
    taskmanager.cpp
    taskmanager.h
    treelandwindow.cpp
    treelandwindow.h
    treelandwindowmonitor.cpp
    treelandwindowmonitor.h
    taskmanagersettings.cpp
    taskmanagersettings.h
)

qt_generate_wayland_protocol_client_sources(dock-taskmanager
    FILES
        ${TREELAND_PROTOCOLS_DATA_DIR}/treeland-foreign-toplevel-manager-v1.xml
        ${WaylandProtocols_DATADIR}/staging/ext-foreign-toplevel-list/ext-foreign-toplevel-list-v1.xml
)

target_link_libraries(dock-taskmanager PRIVATE
    ocean-shell-frame
    Qt${QT_VERSION_MAJOR}::WaylandClientPrivate
    yaml-cpp
    PkgConfig::WaylandClient
)

if (BUILD_WITH_X11)
    target_compile_definitions(dock-taskmanager PRIVATE BUILD_WITH_X11=)
    pkg_check_modules(TaskmanagerXcb REQUIRED IMPORTED_TARGET xcb xcb-res xcb-ewmh xcb-icccm)
    find_package(Dtk${DTK_VERSION_MAJOR} REQUIRED COMPONENTS Widget)
    target_sources(dock-taskmanager PRIVATE
        x11preview.h
        x11preview.cpp
        x11preview.qrc
        x11utils.cpp
        x11utils.h
        x11window.cpp
        x11window.h
        x11windowmonitor.cpp
        x11windowmonitor.h
    )

    target_link_libraries(dock-taskmanager PUBLIC
        PkgConfig::TaskmanagerXcb
        Qt${QT_VERSION_MAJOR}::Widgets
        Dtk${DTK_VERSION_MAJOR}::Widget
    )
endif(BUILD_WITH_X11)

ds_install_package(PACKAGE org.lingmo.ds.dock.taskmanager TARGET dock-taskmanager)
dtk_add_config_meta_files(APPID org.lingmo.ds.dock FILES dconfig/org.lingmo.ds.dock.taskmanager.json) # compat
dtk_add_config_meta_files(APPID org.lingmo.ocean.shell FILES dconfig/org.lingmo.ds.dock.taskmanager.json)
ds_handle_package_translation(PACKAGE org.lingmo.ds.dock.taskmanager)
