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
    property alias cursorPosition: preeditText.cursorPosition   
    property alias text: preeditText.text
    width: parent.width
    height: virtualKeyboard.preeditHeight
    color: virtualKeyboard.virtualKeyboardColor
    x: virtualKeyboard.preeditX

    TextInput {
        id: preeditText
        font.pointSize: virtualKeyboard.preeditTextFontSize
        anchors.verticalCenter: parent.verticalCenter
        color: "black"
        cursorVisible: text.length != 0

        // 暂不支持通过点击改变光标位置
        MouseArea {
            anchors.fill: parent
        }        
    }
}
