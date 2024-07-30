/*
    SPDX-FileCopyrightText: 2015 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.1
import org.kde.kquickcontrols 2.0

Rectangle {
    width: 300
    height: 300
    color: "white"

    ColorButton {
        id: btn

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.margins: 10

        dialogTitle: "I am title"
        showAlphaChannel: true

        color: "blue" //a default colour
    }


    Rectangle {
        anchors.top: parent.top
        anchors.left: btn.right
        anchors.margins: 10

        width: 20
        height: 20

        color: btn.color
    }


}
