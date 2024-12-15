// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15

import org.lingmo.ds 1.0
import org.lingmo.ds.notification 1.0
import org.lingmo.dtk 1.0 as D

NotifyItemContent {
    id: control
    property var bubble

    width: 360
    appName: bubble.appName
    iconName: bubble.iconName
    date: bubble.timeTip
    actions: bubble.actions
    title: bubble.summary
    content: bubble.body
    strongInteractive: bubble.urgency === 2
    contentIcon: bubble.bodyImagePath
    contentRowCount: bubble.contentRowCount
    onRemove: function () {
        console.log("remove notify", bubble.appName)
        Applet.close(bubble.index)
    }
    onActionInvoked: function (actionId) {
        console.log("action notify", bubble.appName, actionId)
        Applet.invokeAction(bubble.index, actionId)
    }

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        z: -1 // default action
        onClicked: {
            if (!bubble.defaultAction)
                return

            console.log("default action notify", bubble.appName, bubble.defaultAction)
            Applet.invokeAction(bubble.index, bubble.defaultAction)
        }
        property bool longPressed
        onPressAndHold: {
            longPressed = true
        }
        onPositionChanged: {
            if (longPressed) {
                longPressed = false
                console.log("delay process", bubble.index)
                Applet.delayProcess(bubble.index)
            }
        }
    }
}
