# SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

find_package(Dtk6 REQUIRED COMPONENTS Core Tools)
add_library(dde-appearance SHARED
    appearanceapplet.cpp
    appearanceapplet.h
)

target_link_libraries(dde-appearance PRIVATE
    dde-shell-frame
    Qt${QT_MAJOR_VERSION}::DBus
)

dtk_add_dbus_interface(DBusInterface org.deepin.dde.Appearance1.xml Appearance1)
target_sources(dde-appearance PUBLIC ${DBusInterface})

ds_install_package(PACKAGE org.deepin.ds.dde-appearance TARGET dde-appearance)
