# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

add_library(osd-brightness SHARED
    brightnessapplet.cpp
    brightnessapplet.h
)

target_link_libraries(osd-brightness PRIVATE
    dde-shell-frame
    Qt${QT_MAJOR_VERSION}::DBus
)

ds_install_package(PACKAGE org.deepin.ds.osd.brightness TARGET osd-brightness)
