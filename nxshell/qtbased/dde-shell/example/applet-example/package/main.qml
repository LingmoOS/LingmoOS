// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Controls 2.4

import org.deepin.ds 1.0
import "control"

AppletItem {
    objectName: "appplet item"
    implicitWidth: 100
    implicitHeight: 100
    Rectangle {
        anchors.fill: parent
        color: "green"
        TextEx {
            anchors.centerIn: parent
            text: "Hello world \n" + String(Applet.id).slice(1, 16)
        }
    }
}
