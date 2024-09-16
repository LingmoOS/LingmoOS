// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import org.deepin.ds 1.0
import org.deepin.dtk 1.0 as D

D.Control {
    id: control
    property var bubble

    contentItem: ColumnLayout {
        spacing: 0
        Rectangle {
            Layout.bottomMargin: -30
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredWidth: bubbleContent.width -  2 * 18
            Layout.preferredHeight: 36
            radius: 18
            opacity: 0.8
            z: control.z + control.z + 1
        }
        NormalBubble {
            id: bubbleContent
            Layout.fillWidth: true
            Layout.maximumWidth: 340
            bubble: control.bubble
        }
    }

    z: bubble.level <= 1 ? 0 : 1 - bubble.level

    background: Rectangle {
        implicitWidth: 200
        radius: 18
        opacity: 1
        color: "transparent"
    }
}
