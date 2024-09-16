# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

add_library(osd-displaymode SHARED
    displaymodeapplet.cpp
    displaymodeapplet.h
)

target_link_libraries(osd-displaymode PRIVATE
    dde-shell-frame
    Qt${QT_MAJOR_VERSION}::DBus
)

ds_install_package(PACKAGE org.deepin.ds.osd.displaymode TARGET osd-displaymode)
ds_handle_package_translation(PACKAGE org.deepin.ds.osd.displaymode)
