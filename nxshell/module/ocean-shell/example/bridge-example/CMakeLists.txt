# SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

add_library(shell-apps-bridge-example SHARED
    exampleappletitem.cpp
    exampleappletitem.h
)

target_link_libraries(shell-apps-bridge-example PRIVATE
    ocean-shell-frame
)

ds_install_package(PACKAGE org.lingmo.ds.example.bridge TARGET shell-apps-bridge-example)
