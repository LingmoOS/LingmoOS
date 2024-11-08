/*
 * Copyright (C) 2021 LingmoOS Team.
 *
 * Author:     revenmartin <revenmartin@gmail.com>
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
import LingmoUI 1.0 as LingmoUI

Item {
    id: control
    width: 24
    height: 24

    property alias source: _image.source
    property color backgroundColor: LingmoUI.Theme.darkMode ? Qt.lighter(LingmoUI.Theme.secondBackgroundColor, 1.3)
                                                          : LingmoUI.Theme.secondBackgroundColor
    property color hoveredColor: LingmoUI.Theme.darkMode ? Qt.lighter(LingmoUI.Theme.secondBackgroundColor, 1.7)
                                                   : Qt.darker(LingmoUI.Theme.secondBackgroundColor, 1.2)
    property color pressedColor: LingmoUI.Theme.darkMode ? Qt.lighter(LingmoUI.Theme.secondBackgroundColor, 1.4)
                                                     : Qt.darker(LingmoUI.Theme.secondBackgroundColor, 1.3)

    signal clicked()

    Rectangle {
        id: _background
        anchors.fill: parent
        radius: LingmoUI.Theme.smallRadius
        color: _mouseArea.pressed ? pressedColor : _mouseArea.containsMouse ? control.hoveredColor : control.backgroundColor
    }

    Image {
        id: _image
        anchors.centerIn: parent
        width: 18
        height: width
        sourceSize: Qt.size(width, height)
        smooth: false
        antialiasing: true
    }

    MouseArea {
        id: _mouseArea
        anchors.fill: parent
        hoverEnabled: true
        acceptedButtons: Qt.LeftButton
        onClicked: control.clicked()
    }
}
