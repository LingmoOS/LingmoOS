# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

find_package(PkgConfig REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} ${REQUIRED_QT_VERSION} COMPONENTS WaylandClient)
find_package(TreelandProtocols REQUIRED)
pkg_check_modules(WaylandClient REQUIRED IMPORTED_TARGET wayland-client)

add_library(dock-showdesktop SHARED
    showdesktop.cpp
    showdesktop.h
    treelandwindowmanager.cpp
    treelandwindowmanager.h
)

qt_generate_wayland_protocol_client_sources(dock-showdesktop
    FILES
        ${TREELAND_PROTOCOLS_DATA_DIR}/treeland-window-management-v1.xml
)

target_link_libraries(dock-showdesktop PRIVATE
    ocean-shell-frame
    Qt${QT_VERSION_MAJOR}::WaylandClient
    PkgConfig::WaylandClient
)

ds_install_package(PACKAGE org.lingmo.ds.dock.showdesktop TARGET dock-showdesktop)
ds_handle_package_translation(PACKAGE org.lingmo.ds.dock.showdesktop)
