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

Rectangle {
    width: parent.width
    height: virtualKeyboard.dragBarHeight
    color: virtualKeyboard.virtualKeyboardColor
    radius: virtualKeyboard.virtualKeyboardFloatPlacementRadius

    property int startX: 0;
        property int startY: 0;

            MouseArea {
                anchors.fill: parent
                onPressed: {
                    startX = mouseX
                    startY = mouseY
                }
                onPositionChanged: {
                    virtualKeyboard.moveBy(mouseX - startX, mouseY - startY)
                }

                onReleased: {
                    virtualKeyboard.endDrag()
                }
            }

            Rectangle {
                id: dragBarIndicator
                anchors.centerIn: parent
                width: virtualKeyboard.dragBarIndicatorWidth
                height: virtualKeyboard.dragBarIndicatorHeight
                color: virtualKeyboard.dragBarIndicatorColor
                radius: virtualKeyboard.dragBarIndicatorRadius
            }

            state: virtualKeyboard.placementMode
            states: [
                State {
                    name: "EXPANSION"
                    PropertyChanges {
                        target: dragBar
                        visible: false
                    }
                },
                State {
                    name: "FLOAT"
                    PropertyChanges {
                        target: dragBar
                        visible: true
                    }
                }
            ]
        }
