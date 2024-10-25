/*
    SPDX-FileCopyrightText: 2011 Daker Fernandes Pinheiro <dakerfp@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import org.kde.lingmo.core
import org.kde.lingmo.components

FrameSvgItem {
    width: 300
    height: 400
    imagePath: "widgets/background"


    ToolBar {
        id: toolBar
        z: 10

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            margins: 10
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
            margins: 10
        }
        initialPage: Qt.createComponent("Menu.qml")
    }
}
