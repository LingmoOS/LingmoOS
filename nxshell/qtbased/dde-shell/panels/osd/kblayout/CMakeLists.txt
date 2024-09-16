# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

add_library(osd-kblayout SHARED
    kblayoutapplet.cpp
    kblayoutapplet.h
)

target_link_libraries(osd-kblayout PRIVATE
    dde-shell-frame
)

ds_install_package(PACKAGE org.deepin.ds.osd.kblayout TARGET osd-kblayout)
