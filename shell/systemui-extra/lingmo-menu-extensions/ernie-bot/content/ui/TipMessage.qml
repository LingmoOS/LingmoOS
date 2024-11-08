/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: hxf <hewenfei@kylinos.cn>
 */

import QtQuick 2.12
import QtQuick.Layouts 1.12

Rectangle {
    readonly property int padding: 16
    height: tipLayout.height + padding*2
    radius: 8
    color: Qt.rgba(255, 255, 255, 0.45)
    border.width: 1
    border.color: Qt.rgba(0, 0, 0, 0.1)

    ColumnLayout {
        id: tipLayout
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.leftMargin: parent.padding
        anchors.rightMargin: parent.padding
        anchors.topMargin: parent.padding

        spacing: 8
        onChildrenRectChanged: {
            height = childrenRect.height;
        }

        Text {
            Layout.fillWidth: true
            Layout.preferredHeight: contentHeight
            wrapMode: Text.WrapAnywhere
            font.bold: true
            text: header
        }

        Text {
            Layout.fillWidth: true
            Layout.preferredHeight: contentHeight
            wrapMode: Text.WrapAnywhere
            text: content
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            Layout.maximumHeight: 1

            color: Qt.rgba(38, 38, 38, 0.2)
        }

        ListView {
            Layout.fillWidth: true
            Layout.leftMargin: 8
            Layout.rightMargin: 8
            Layout.preferredHeight: contentHeight

            cacheBuffer: count
            interactive: false
            spacing: 8
            model: actions
            delegate: Item {
                width: ListView.view.width
                height: 32

                RowLayout {
                    anchors.fill: parent

                    Text {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        verticalAlignment: Text.AlignVCenter
                        elide: Text.ElideMiddle

                        text: "\"" + modelData.text + "\""
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.maximumWidth: 70
                        Layout.minimumWidth: 70
                        Layout.fillHeight: true

                        radius: 8
                        color: "#263790FA"

                        Text {
                            anchors.fill: parent
                            text: modelData.name
                            verticalAlignment: Text.AlignVCenter
                            horizontalAlignment: Text.AlignHCenter
                            elide: Text.ElideRight
                            color: "#3790FA"
                        }

                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                modelData.exec();
                            }
                        }
                    }
                }
            }
        }
    }
}
