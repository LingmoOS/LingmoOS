# SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

add_library(dde-am SHARED
    amapplet.cpp
    amapplet.h
)

target_link_libraries(dde-am PRIVATE
    dde-shell-frame
    Qt${QT_MAJOR_VERSION}::DBus
)

ds_install_package(PACKAGE org.deepin.ds.dde-am TARGET dde-am)
