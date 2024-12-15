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

    states: [
        State {
            name: "removing"
            PropertyChanges { target: root; x: root.width; opacity: 0}
        }
    ]

    transitions: Transition {
        to: "removing"
        ParallelAnimation {
            NumberAnimation { properties: "x"; duration: 300; easing.type: Easing.Linear }
            NumberAnimation { properties: "opacity"; duration: 300; easing.type: Easing.Linear }
        }
        onRunningChanged: {
            if (!running) {
                root.remove()
            }
        }
    }

    contentItem: NotifyItemContent {
        width: parent.width
        appName: root.appName
        iconName: root.iconName
        content: root.content
        title: root.title
        date: root.date
        actions: root.actions
        closeVisible: root.hovered || root.activeFocus
        strongInteractive: root.strongInteractive
        contentIcon: root.contentIcon
        contentRowCount: root.contentRowCount

        onRemove: function () {
            root.state = "removing"
        }
        onActionInvoked: function (actionId) {
            root.actionInvoked(actionId)
        }
    }
}
