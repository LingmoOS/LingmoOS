# SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

add_library(dock-clipboarditem SHARED
    clipboarditem.cpp
    clipboarditem.h
    ../dockiteminfo.cpp
    ../dockiteminfo.h
)

target_include_directories(dock-clipboarditem PRIVATE
    "${CMAKE_CURRENT_BINARY_DIR}/../"
    "../"
)

target_link_libraries(dock-clipboarditem PRIVATE
    dde-shell-frame
)

ds_install_package(PACKAGE org.deepin.ds.dock.clipboarditem TARGET dock-clipboarditem)
ds_handle_package_translation(PACKAGE org.deepin.ds.dock.clipboarditem)

install(FILES "package/icons/clipboard.svg" DESTINATION share/dde-dock/icons/dcc-setting)
