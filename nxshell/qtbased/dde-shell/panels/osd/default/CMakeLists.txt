# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

add_library(osd-default SHARED
)

target_link_libraries(osd-default PRIVATE
    dde-shell-frame
)

ds_install_package(PACKAGE org.deepin.ds.osd.default)
ds_handle_package_translation(PACKAGE org.deepin.ds.osd.default)
