// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import org.lingmo.ds 1.0
import org.lingmo.ds.notification 1.0
import org.lingmo.dtk 1.0 as D

Item {
    id: control
    property var bubble
    readonly property int radius: 12

    height: bubbleContent.height + indicator.height
    OverlapIndicator {
        id: indicator
        width: parent.width
        count: bubble.level - 1
        revert: true
        anchors {
            top: parent.top
            left: parent.left
            leftMargin: radius
            right: parent.right
            rightMargin: radius
        }
    }

    NormalBubble {
        id: bubbleContent
        width: parent.width
        bubble: control.bubble
        anchors {
            top: indicator.bottom
        }
    }
}
