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

CharKey {
    normalColor: virtualKeyboard.actionKeyNormalColor
    pressedColor: virtualKeyboard.actionKeyPressedColor
    hoverColor: virtualKeyboard.actionKeyHoverColor

    fontSize: virtualKeyboard.actionKeyFontSize

    property string actionKeyImgPath
    keyLabel.visible: actionKeyImgPath == ""

    Loader {
        width: virtualKeyboard.toolbarSize
        height: virtualKeyboard.toolbarSize
        anchors.verticalCenter: parent.verticalCenter
        sourceComponent: actionKeyImgPath == "" ? undefined : actionKeyComponent
        anchors {
            horizontalCenter: alignment == Text.AlignHCenter ? parent.horizontalCenter : undefined
            right: alignment == Text.AlignRight ? parent.right : undefined
            rightMargin: alignment == Text.AlignRight ? virtualKeyboard.keyIconAlignment : undefined
            left: alignment == Text.AlignLeft ? parent.left : undefined
            leftMargin: alignment == Text.AlignLeft ? virtualKeyboard.keyIconAlignment : undefined
        }
    }

    Component {
        id: actionKeyComponent
        Image {
            source: actionKeyImgPath
        }
    }
}
