// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.4
import QtQuick.Window 2.15

import org.deepin.ds 1.0

Window {
    id: root
    visible: true
    height: 200
    DLayerShellWindow.anchors: DLayerShellWindow.AnchorBottom | DLayerShellWindow.AnchorLeft | DLayerShellWindow.AnchorRight

    Repeater {
        anchors.fill: parent
        model: Applet.appletItems
        Control {
            anchors.fill: parent
            padding: 20
            contentItem: model.data
            background: Rectangle {
                color: "plum"
                opacity: 0.8
            }
        }
    }
}
