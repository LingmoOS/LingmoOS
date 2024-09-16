# SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

add_library(dock-searchitem SHARED
    searchitem.cpp
    searchitem.h
    ../dockiteminfo.cpp
    ../dockiteminfo.h
)

target_include_directories(dock-searchitem PRIVATE
    "${CMAKE_CURRENT_BINARY_DIR}/../"
    "../"
)

target_link_libraries(dock-searchitem PRIVATE
    dde-shell-frame
)

ds_install_package(PACKAGE org.deepin.ds.dock.searchitem TARGET dock-searchitem)
ds_handle_package_translation(PACKAGE org.deepin.ds.dock.searchitem)

install(FILES "package/icons/search.svg" DESTINATION share/dde-dock/icons/dcc-setting)
