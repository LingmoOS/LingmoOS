# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

add_library(ds-example-applet-data SHARED
    exampleapplet.h
    exampleapplet.cpp
)

target_link_libraries(ds-example-applet-data PRIVATE
    dde-shell-frame
)

ds_install_package(PACKAGE org.deepin.ds.example.applet-data TARGET ds-example-applet-data)
