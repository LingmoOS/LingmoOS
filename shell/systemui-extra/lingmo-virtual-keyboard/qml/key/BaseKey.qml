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
import QtGraphicalEffects 1.0

ToolButton {
    property int keycode
    width: virtualKeyboard.keyWidth
    height: virtualKeyboard.keyHeight
    property string label: ""
        property real fontSize: virtualKeyboard.fontSize
            property alias keyLabel: keyLabel_
                property alias keyBackground: keyBackground_
                    property alias dropShadow: dropShadow_
                        /*label和img对应父控件的水平对齐方式 取值为AlignHCenter:居中对齐, AlignRight:向右对齐, AlignLeft:向左对齐*/
                        property int alignment: Text.AlignHCenter

                            background: Rectangle {
                                id: keyBackground_
                                radius: virtualKeyboard.keyRadius
                            }

                            DropShadow {
                                id: dropShadow_
                                anchors.fill: parent
                                verticalOffset: virtualKeyboard.dropShadowVerticalOffset
                                radius: virtualKeyboard.keyRadius
                                color: virtualKeyboard.charKeyDropShadowColor
                                source: keyBackground
                                samples: 100
                            }


                            Label {
                                id: keyLabel_
                                text: label
                                color: "black"
                                font.pointSize: fontSize
                                font.weight: Font.Light
                                anchors.fill: parent
                                padding: alignment == Text.AlignHCenter ? undefined : virtualKeyboard.keyLableAlignment
                                horizontalAlignment: alignment
                                verticalAlignment: Text.AlignVCenter
                                visible: true
                            }
                        }

