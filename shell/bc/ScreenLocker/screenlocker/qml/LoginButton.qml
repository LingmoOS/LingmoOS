/*
 * Copyright (C) 2021 CuteOS Team.
 *
 * Author:     Reion Wong <reion@cuteos.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.12
import QtQuick.Window 2.3
import QtQuick.Controls 2.4
import LingmoUI 1.0 as CuteUI

Item {
    id: control

    property var size: 32
    property var iconMargins: 0
    height: size
    width: size

    property alias background: _background
    property color backgroundColor: CuteUI.Theme.highlightColor
    property color hoveredColor: Qt.lighter(backgroundColor, 1.1)
    property color pressedColor: Qt.darker(backgroundColor, 1.2)
    property alias source: _image.source
    property alias image: _image
    signal clicked()

    Rectangle {
        id: _background
        anchors.fill: parent
        // radius: CuteUI.Theme.bigRadius //control.height / 2
        color: mouseArea.pressed ? pressedColor : mouseArea.containsMouse ? control.hoveredColor : control.backgroundColor

        gradient: Gradient {
            GradientStop { position: 0.0; color: Qt.lighter(_background.color, 1.3) }
            GradientStop { position: 1.0; color: _background.color }
        }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        onClicked: control.clicked()
    }

    Image {
        id: _image
        objectName: "image"
        anchors.fill: parent
        anchors.margins: control.iconMargins
        fillMode: Image.PreserveAspectFit
        sourceSize: Qt.size(width, height)
        cache: true
        asynchronous: false
    }
}
