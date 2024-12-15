# SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

add_library(ocean-am SHARED
    amapplet.cpp
    amapplet.h
)

target_link_libraries(ocean-am PRIVATE
    ocean-shell-frame
    Qt${QT_MAJOR_VERSION}::DBus
)

ds_install_package(PACKAGE org.lingmo.ds.ocean-am TARGET ocean-am)
