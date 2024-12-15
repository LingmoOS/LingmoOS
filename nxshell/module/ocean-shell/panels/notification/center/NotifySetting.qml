// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import org.lingmo.dtk 1.0
import org.lingmo.ds.notificationcenter

NotifySettingMenu {
    id: root

    required property NotifyModel notifyModel
    property bool pinned
    property string appName

    MenuItem {
        text: pinned ? qsTr("Unpin") : qsTr("Pin")
        onClicked: {
            let state = !root.pinned
            console.log("Pin changed", state)
            notifyModel.pinApplication(appName, state)
        }
    }
    MenuItem {
        text: qsTr("Notification Setting")
        onClicked: {
            console.log("Notify setting")
            NotifyAccessor.openNotificationSetting()
        }
    }
}
