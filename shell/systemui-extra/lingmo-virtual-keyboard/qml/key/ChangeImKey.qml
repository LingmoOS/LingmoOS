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
import QtQuick.Controls 2.12
import QtQml 2.1
import "../js/utils.js" as Utils

SwitchKey {
    id: changeIm
    label: virtualKeyboard.currentIM.split("|")[2]

    Binding {
        target: changeIm
        property: "switchKeyState"
        value: virtualKeyboard.changeIMState
    }

    Connections {
        target: virtualKeyboard
        onQmlImDeactivated: {
            imList.close()
        }
    }

    multiPointTouchArea.onReleased: {
        if(virtualKeyboard.changeIMState == "NORMAL"){
            virtualKeyboard.changeIMState = "OPEN"
            imList.open()
        }else {
            imList.close()
        }
    }

    multiPointTouchArea.onPressed: {
        imList.currentIndex =
                virtualKeyboard.currentIMList.indexOf(virtualKeyboard.currentIM)
    }

    state: virtualKeyboard.changeIMState
    states: [
        State {
            name: "NORMAL"
            PropertyChanges {
                target: keyBackground
                state: "NORMAL"
            }
            PropertyChanges {
                target: virtualKeyboard
                isCurrentIMListVisible : false
            }
        },
        State {
            name: "OPEN"
            PropertyChanges {
                target: keyBackground
                state: "OPEN"
            }
            PropertyChanges {
                target: virtualKeyboard
                isCurrentIMListVisible : true
            }
        }
    ]

    Rectangle {
        id: imListRectangle
        anchors.bottom: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        visible: virtualKeyboard.isCurrentIMListVisible
        width: imList.width
        height: imList.height
        radius: 8
        Menu {
            id: imList

            width: virtualKeyboard.imListItemWidth
            // 输入法列表的最大高度可以完整显示四个条目，
            // 输入法列表条目数量超过4之后输入法列表显示滚动条
            height: imList.count == 0 ? 0 : imList.itemAt(0).height * Math.min(imList.count, 4)
            modal: true
            Overlay.modal:Rectangle {
                color: "transparent"
            }
            background: Rectangle {
                radius: imListRectangle.radius
                width: parent.width
                height: parent.height
                color: "white"
            }

            Instantiator {
                model: virtualKeyboard.currentIMList
                delegate: MenuItem {
                    background: Rectangle {
                        radius: imListRectangle.radius
                        color: highlighted? virtualKeyboard.currentIMColor: "white"
                    }
                    property string uniqueName: modelData.split("|")[0]
                    property string localName: modelData.split("|")[1]
                    property string label: modelData.split("|")[2]
                    font.pointSize: virtualKeyboard.imListFontSize
                    text: label.padEnd(3, " ") + localName
                    width: virtualKeyboard.imListItemWidth
                    height: virtualKeyboard.imListItemHeight
                    MouseArea {
                        anchors.fill: parent
                        onReleased: {
                            virtualKeyboard.setCurrentIM(uniqueName)
                            imList.close()
                        }
                    }
                }

                onObjectAdded: imList.insertItem(index, object)
                onObjectRemoved: imList.removeItem(object)
            }

            onClosed: {
                virtualKeyboard.changeIMState = "NORMAL"
            }
        }
    }

}