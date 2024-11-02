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
    id: root
    border.width: 1
    border.color: "#dedede"

    property bool disableInput: false
    signal messageSend(string message)

    function sendMessage(message) {
        if (message.length > 0) {
            messageSend(message);
            messageInput.clear();
        }
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.leftMargin: 8
            Layout.rightMargin: 4
            clip: true

            TextInput {
                id: messageInput
                anchors.fill: parent
                z: 10
                verticalAlignment: TextInput.AlignVCenter
                selectByMouse: true
                selectionColor: "#3790FA"
                maximumLength: 500
                rightPadding: 4
                readOnly: root.disableInput

                onAccepted: {
                    root.sendMessage(text);
                }
            }

            Text {
                anchors.fill: parent
                visible: messageInput.length === 0
                verticalAlignment: Text.AlignVCenter
                color: Qt.rgba(0, 0, 0, 0.3)
                text: qsTr("You can ask me any questions~")
            }
        }

        Rectangle {
            Layout.fillHeight: true
            Layout.preferredWidth: 1

            color: "#dedede"
        }

        Item {
            Layout.fillHeight: true
            Layout.preferredWidth: parent.height

            Image {
                width: 16
                height: 16
                anchors.centerIn: parent
                enabled: !root.disableInput;
                source: "qrc:///ernie-bot/content/icon/send.svg"
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    root.sendMessage(messageInput.text);
                }
            }
        }
    }
}
