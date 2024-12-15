// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.lingmo.dtk 1.0
import org.lingmo.ds.notification
import org.lingmo.ds.notificationcenter

NotifyItem {
    id: root

    signal collapse()

    contentItem: RowLayout {
        Text {
            text: root.appName
            Layout.alignment: Qt.AlignLeft
            Layout.leftMargin: 18
            font: DTK.fontManager.t5
            color: palette.windowText
        }

        Item {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
        }

        SettingActionButton {
            Layout.alignment: Qt.AlignRight
            icon.name: "fold"
            onClicked: {
                console.log("collapse")
                root.collapse()
            }
        }
        SettingActionButton {
            Layout.alignment: Qt.AlignRight
            icon.name: "more"
            onClicked: function () {
                console.log("group setting", appName)
                let pos = mapToItem(root, Qt.point(width / 2, height))
                root.setting(pos)
            }
        }
        AnimationSettingButton {
            Layout.alignment: Qt.AlignRight
            icon.name: "clean-group"
            text: qsTr("Clear All")
            onClicked: function () {
                root.remove()
            }
        }
    }
}
