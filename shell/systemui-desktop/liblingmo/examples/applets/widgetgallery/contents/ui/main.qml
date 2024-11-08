/*
    SPDX-FileCopyrightText: 2011 Daker Fernandes Pinheiro <dakerfp@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import org.kde.lingmo.plasmoid
import QtQuick.Layouts
import org.kde.lingmo.components

Item {
    Plasmoid.switchWidth: 300
    Plasmoid.switchHeight: 400

    Plasmoid.fullRepresentation: Item {
        Layout.minimumWidth: 300
        Layout.minimumHeight: 400

        ToolBar {
            id: toolBar
            z: 10
            anchors {
                top: parent.top
                left: parent.left
                right: parent.right
            }
        }

        PageStack {
            id: pageStack
            toolBar: toolBar
            clip: true
            anchors {
                top: toolBar.bottom
                left: parent.left
                right: parent.right
                bottom: parent.bottom
            }
            initialPage: Qt.createComponent("Menu.qml")
        }
    }
}
