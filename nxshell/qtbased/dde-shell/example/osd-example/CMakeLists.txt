# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

add_library(ds-osd-example SHARED
    osdapplet.h
    osdapplet.cpp
)

target_link_libraries(ds-osd-example PRIVATE
    dde-shell-frame
)

ds_install_package(PACKAGE org.deepin.ds.example.osd TARGET ds-osd-example)
