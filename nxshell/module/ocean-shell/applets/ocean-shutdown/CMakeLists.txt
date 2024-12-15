# SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later
find_package(TreelandProtocols REQUIRED)

add_library(ocean-shutdown SHARED
    shutdownapplet.cpp
    shutdownapplet.h
    treelandlockscreen.h
    treelandlockscreen.cpp
)

qt_generate_wayland_protocol_client_sources(ocean-shutdown
    FILES
        ${TREELAND_PROTOCOLS_DATA_DIR}/treeland-ocean-shell-v1.xml
)

target_link_libraries(ocean-shutdown PRIVATE
    ocean-shell-frame
    Qt${QT_MAJOR_VERSION}::DBus
    Qt${QT_VERSION_MAJOR}::WaylandClient
)

ds_install_package(PACKAGE org.lingmo.ds.ocean-shutdown TARGET ocean-shutdown)
