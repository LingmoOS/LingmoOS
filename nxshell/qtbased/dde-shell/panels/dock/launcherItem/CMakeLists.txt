# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

add_library(dock-launcheritem SHARED
    launcheritem.cpp
    launcheritem.h
)

target_link_libraries(dock-launcheritem PRIVATE
    dde-shell-frame
)

ds_install_package(PACKAGE org.deepin.ds.dock.launcheritem TARGET dock-launcheritem)
ds_handle_package_translation(PACKAGE org.deepin.ds.dock.launcheritem)
