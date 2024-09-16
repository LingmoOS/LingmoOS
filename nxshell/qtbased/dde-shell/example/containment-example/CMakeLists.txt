# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

add_library(ds-example-containment SHARED
    examplecontainment.h
    examplecontainment.cpp
)

target_link_libraries(ds-example-containment PRIVATE
    dde-shell-frame
)

ds_install_package(PACKAGE org.deepin.ds.example.containment TARGET ds-example-containment)
