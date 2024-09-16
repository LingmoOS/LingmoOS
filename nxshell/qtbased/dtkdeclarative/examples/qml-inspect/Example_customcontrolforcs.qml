// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.0
import org.deepin.dtk 1.0

Control {
    id: control
    hoverEnabled: true
    property Palette backgroundColor: Palette {
        normal: Qt.hsla(Math.random(), 0.2, 0.5, 0.9)
        hovered: Qt.hsla(Math.random(), 0.1, 0.1, 0.9)
    }
    property Palette textColor: Palette {
        normal: Qt.hsla(Math.random(), 0.5, 0.2, 0.4)
        hovered: Qt.hsla(Math.random(), 0.6, 0.6, 0.9)
    }

    property string text

    background: Rectangle {
        color: control.ColorSelector.backgroundColor
        Behavior on color {
            ColorAnimation {
                duration: 300
            }
        }
    }

    contentItem: Text {
        id: textItem
        anchors.fill: parent
        horizontalAlignment: Qt.AlignHCenter
        verticalAlignment: Qt.AlignVCenter
        text: control.text
        color: control.ColorSelector.textColor

        Behavior on color {
            ColorAnimation {
                duration: 300
            }
        }
    }
}
