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
import "../js/utils.js" as Utils

BaseKey {

    property color normalColor: virtualKeyboard.switchKeyNormalColor
    property color pressedColor: virtualKeyboard.switchKeyPressedColor
    property color openColor: virtualKeyboard.switchKeyOpenColor
    property color openPressedColor: virtualKeyboard.switchKeyOpenPressedColor
    property color hoverNormalColor: virtualKeyboard.switchKeyHoverNormalColor
    property color hoverOpenColor: virtualKeyboard.switchKeyHoverOpenColor
    property alias multiPointTouchArea: multiPointTouchArea_
    property string switchKeyState

    fontSize: virtualKeyboard.switchKeyFontSize

    keyBackground.state: "NORMAL"
    keyBackground.states: [
        State {
            name: "NORMAL"
            PropertyChanges {
                target: keyBackground
                color: normalColor
            }
            PropertyChanges {
                target: dropShadow
                verticalOffset: virtualKeyboard.dropShadowVerticalOffset
                color: virtualKeyboard.switchKeyNormalDropShadowColor
            }
        },
        State {
            name: "PRESSED"
            PropertyChanges {
                target: keyBackground
                color: pressedColor
            }
            PropertyChanges {
                target: dropShadow
                verticalOffset: -virtualKeyboard.dropShadowVerticalOffset
                color: virtualKeyboard.switchKeyNormalDropShadowColor
            }
        },
        State {
            name: "OPEN"
            PropertyChanges {
                target: keyBackground
                color: openColor
            }
            PropertyChanges {
                target: dropShadow
                verticalOffset: -virtualKeyboard.dropShadowVerticalOffset
                color: virtualKeyboard.switchKeyOpenDropShadowColor
            }
            PropertyChanges {
                target: keyLabel
                color: "white"
            }
        },
        State {
            name: "OPEN_PRESSED"
            PropertyChanges {
                target: keyBackground
                color: openPressedColor
            }
            PropertyChanges {
                target: dropShadow
                verticalOffset: -virtualKeyboard.dropShadowVerticalOffset
                color: virtualKeyboard.switchKeyOpenPressedDropShadowColor
            }
            PropertyChanges {
                target: keyLabel
                color: "white"
            }
        },
        State {
            name: "HOVER_NORMAL"
            PropertyChanges {
                target: keyBackground
                color: hoverNormalColor
            }
        },
        State {
            name: "HOVER_OPEN"
            PropertyChanges {
                target: keyBackground
                color: hoverOpenColor
            }
            PropertyChanges {
                target: dropShadow
                verticalOffset: -virtualKeyboard.dropShadowVerticalOffset
                color: virtualKeyboard.switchKeyOpenDropShadowColor
            }
            PropertyChanges {
                target: keyLabel
                color: "white"
            }
        }
    ]

    MouseArea {
        id: keyMouseArea_
        anchors.fill: parent
        hoverEnabled: !virtualKeyboard.isShiftKeyLongPressed
        onEntered: {
            if (switchKeyState == "NORMAL") {
                keyBackground.state = "HOVER_NORMAL"
            } else if (switchKeyState == "OPEN") {
                keyBackground.state = "HOVER_OPEN"
            }
        }

        onExited: {
            if (switchKeyState == "NORMAL") {
                keyBackground.state = "NORMAL"
            } else if (switchKeyState == "OPEN") {
                keyBackground.state = "OPEN"
            }
        }
    }

    MultiPointTouchArea {
        id: multiPointTouchArea_
        anchors.fill: parent
        maximumTouchPoints: 1
        onPressed: {
            if (switchKeyState == "NORMAL") {
                keyBackground.state = "PRESSED"
            } else {
                keyBackground.state = "OPEN_PRESSED"
            }
        }
    }
}
