// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Window 2.11
import org.deepin.dtk 1.0 as D

Rectangle {
    id: control
    width: 200; height: 200
    color: "red"
    Window {
        // on top of the Rectangle's window
        x: control.Window.window ? control.Window.window.x : 0
        y: control.Window.window ? control.Window.window.y : 0
        width: 100; height: 100
        visible: true

        D.DWindow.enabled: true
        D.DWindow.alphaBufferSize: 8

        D.BehindWindowBlur {
            anchors.fill: parent
            blendColor: Qt.rgba(0, 255, 0, 0.4)
            cornerRadius: 10
        }
    }
}
