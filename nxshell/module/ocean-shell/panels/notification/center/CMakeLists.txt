# SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

qt_add_qml_module(notificationcenterpanel
    PLUGIN_TARGET notificationcenterpanelplugin
    URI org.lingmo.ds.notificationcenter
    VERSION 1.0
    QML_FILES
        NotifyCenter.qml
        NotifyStaging.qml
        NotifyHeader.qml
        NotifyView.qml
        NotifyViewDelegate.qml
        NormalNotify.qml
        OverlapNotify.qml
        GroupNotify.qml
        NotifySetting.qml
        NotifySettingMenu.qml
        AnimationSettingButton.qml
        BoundingRectangle.qml
        DataPanel.qml
    SOURCES
        notificationcenterpanel.h
        notificationcenterpanel.cpp
        notificationcenterproxy.h
        notificationcenterproxy.cpp
        notificationcenterdbusadaptor.h
        notificationcenterdbusadaptor.cpp
        notifyaccessor.h
        notifyaccessor.cpp
        notifymodel.h
        notifymodel.cpp
        notifyitem.h
        notifyitem.cpp
        notifystagingmodel.h
        notifystagingmodel.cpp
    OUTPUT_DIRECTORY
        ${PROJECT_BINARY_DIR}/plugins/org/lingmo/ds/notificationcenter/
)

qt_add_resources(notificationcenterpanel "ds_notificationcenterpanel_icons"
    PREFIX "/dsg/built-in-icons"
    BASE "icons"
    FILES
        icons/clean-group.dci
        icons/clean-alone.dci
        icons/clean-all.dci
        icons/fold.dci
        icons/more.dci
)

target_link_libraries(notificationcenterpanel
    PRIVATE
        ocean-shell-frame
        ds-notification-shared
        Qt${QT_VERSION_MAJOR}::Quick
        Qt${QT_VERSION_MAJOR}::DBus
)

install(TARGETS notificationcenterpanelplugin DESTINATION "${QML_INSTALL_DIR}/org/lingmo/ds/notificationcenter/")
install(DIRECTORY "${PROJECT_BINARY_DIR}/plugins/org/lingmo/ds/notificationcenter/" DESTINATION "${QML_INSTALL_DIR}/org/lingmo/ds/notificationcenter/")

ds_install_package(PACKAGE org.lingmo.ds.notificationcenter TARGET notificationcenterpanel)
ds_handle_package_translation(PACKAGE org.lingmo.ds.notificationcenter)
