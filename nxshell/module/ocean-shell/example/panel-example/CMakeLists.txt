# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

add_library(ds-panel-example SHARED
    examplepanel.cpp
    examplepanel.h
)

target_link_libraries(ds-panel-example PRIVATE
    ocean-shell-frame
)

find_package(Dtk${DTK_VERSION_MAJOR}DConfig REQUIRED)

dtk_add_config_meta_files(APPID "org.lingmo.ocean.shell" FILES org.lingmo.ds.example.json)

ds_install_package(PACKAGE org.lingmo.ds.example TARGET ds-panel-example)
