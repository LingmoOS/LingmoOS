# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

add_library(ds-panel-example SHARED
    examplepanel.cpp
    examplepanel.h
)

target_link_libraries(ds-panel-example PRIVATE
    dde-shell-frame
)

find_package(Dtk${DTK_VERSION_MAJOR}DConfig REQUIRED)

dtk_add_config_meta_files(APPID "org.deepin.dde.shell" FILES org.deepin.ds.example.json)

ds_install_package(PACKAGE org.deepin.ds.example TARGET ds-panel-example)
