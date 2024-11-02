/*
* Copyright 2022 KylinSoft Co., Ltd.
*
* This program is free software: you can redistribute it and/or modify it under
* the terms of the GNU General Public License as published by the Free Software
* Foundation, either version 3 of the License, or (at your option) any later
* version.
*
* This program is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
* FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
* details.
*
* You should have received a copy of the GNU General Public License along with
* this program. If not, see <https://www.gnu.org/licenses/>.
*/

import QtQuick 2.0
import QtQuick.Controls 2.0

Rectangle {
    height: virtualKeyboard.toolbarSize
    width: virtualKeyboard.toolbarSize
    anchors.verticalCenter: parent.verticalCenter
    anchors.right: parent.right
    anchors.rightMargin: virtualKeyboard.cardinalNumber * 3.5
    color: virtualKeyboard.virtualKeyboardColor

    Image {
        id: hideButtonImg
        anchors.centerIn: parent
        sourceSize: Qt.size(parent.width, parent.width)
        source: "qrc:/img/close.svg"
    }

    ToolTip {
        id:hideButtonToolTip
        text: qsTr("close")
    }

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true

        onPressed: {
            hideButtonImg.source = "qrc:/img/close_pressed.svg"
        }

        onReleased: {
            virtualKeyboard.hideVirtualKeyboard()
        }

        onEntered: {
            hideButtonToolTip.visible = true
        }

        onExited: {
            hideButtonToolTip.visible = false
        }
    }
}
