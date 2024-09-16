# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: CC0-1.0

add_subdirectory(quickpanel)

find_package(Qt${QT_VERSION_MAJOR} ${REQUIRED_QT_VERSION} REQUIRED COMPONENTS Gui Qml)
find_package(Dtk6 REQUIRED COMPONENTS Core Gui)
find_package(DdeTrayLoader REQUIRED)

qt_policy(SET QTP0001 OLD)
qt_add_qml_module(dock-tray
    PLUGIN_TARGET dock-tray
    URI "org.deepin.ds.dock.tray"
    VERSION "1.0"
    SHARED
    SOURCES
        traysortordermodel.cpp
        traysortordermodel.h
        trayitempositionregister.cpp
        trayitempositionregister.h
        trayitempositionmanager.cpp
        trayitempositionmanager.h
        ksortfilterproxymodel.cpp
        ksortfilterproxymodel.h
        kextracolumnsproxymodel.cpp
        kextracolumnsproxymodel.h
        listtotableproxymodel.cpp
        listtotableproxymodel.h
    QML_FILES
        SurfacePopup.qml
        SurfaceSubPopup.qml
        TrayItemSurfacePopup.qml
        ShellSurfaceItemProxy.qml

    OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/plugins/org/deepin/ds/dock/tray/
)

target_link_libraries(dock-tray PUBLIC
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Qml
    Dtk${DTK_VERSION_MAJOR}::Core
)

add_library(trayitem SHARED
    trayitem.cpp
    trayitem.h
    traysettings.cpp
    traysettings.h
    ../dockiteminfo.cpp
    ../dockiteminfo.h
    ../constants.h
)

target_include_directories(trayitem PUBLIC
    "${CMAKE_CURRENT_BINARY_DIR}/../"
    "../"
)

target_link_libraries(trayitem PRIVATE
    dde-shell-frame
)

ds_install_package(PACKAGE org.deepin.ds.dock.tray TARGET trayitem)
ds_handle_package_translation(PACKAGE org.deepin.ds.dock.tray)
