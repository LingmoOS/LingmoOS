# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

add_library(notificationpanel SHARED
    notificationpanel.cpp
    notificationpanel.h
    notificationinterproxy.cpp
    notificationinterproxy.h
    bubblemodel.cpp
    bubblemodel.h
    test.sh
)

target_link_libraries(notificationpanel PRIVATE
    dde-shell-frame
    Qt${QT_MAJOR_VERSION}::DBus
)

ds_install_package(PACKAGE org.deepin.ds.notification TARGET notificationpanel)
ds_handle_package_translation(PACKAGE org.deepin.ds.notification)
