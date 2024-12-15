# SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

find_package(Dtk6 REQUIRED COMPONENTS Core Tools)
add_library(ocean-appearance SHARED
    appearanceapplet.cpp
    appearanceapplet.h
)

target_link_libraries(ocean-appearance PRIVATE
    ocean-shell-frame
    Qt${QT_MAJOR_VERSION}::DBus
)

dtk_add_dbus_interface(DBusInterface org.lingmo.ocean.Appearance1.xml Appearance1)
target_sources(ocean-appearance PUBLIC ${DBusInterface})

ds_install_package(PACKAGE org.lingmo.ds.ocean-appearance TARGET ocean-appearance)
