# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

add_library(dock-multitaskview SHARED
    multitaskview.cpp
    multitaskview.h
    ../dockiteminfo.cpp
    ../dockiteminfo.h
)

target_include_directories(dock-multitaskview PRIVATE
    "${CMAKE_CURRENT_BINARY_DIR}/../"
    "../"
)

target_link_libraries(dock-multitaskview PRIVATE
    dde-shell-frame
)

ds_install_package(PACKAGE org.deepin.ds.dock.multitaskview TARGET dock-multitaskview)
ds_handle_package_translation(PACKAGE org.deepin.ds.dock.multitaskview)

install(FILES "package/icons/multitasking-view.svg" DESTINATION share/dde-dock/icons/dcc-setting)
