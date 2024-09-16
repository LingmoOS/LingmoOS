# SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

add_library(dock-workspaceitem SHARED
    workspaceitem.cpp
    workspaceitem.h
    workspacemodel.h
    workspacemodel.cpp
    xworkspaceworker.h
    xworkspaceworker.cpp
    desktops.h
    desktops.cpp
)

target_link_libraries(dock-workspaceitem PRIVATE
    dde-shell-frame
)

ds_install_package(PACKAGE org.deepin.ds.dock.workspaceitem TARGET dock-workspaceitem)
ds_handle_package_translation(PACKAGE org.deepin.ds.dock.workspaceitem)
