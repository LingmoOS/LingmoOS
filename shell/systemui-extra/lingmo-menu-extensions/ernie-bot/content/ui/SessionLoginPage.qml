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

Item {
    Component.onCompleted: {
        extensionData.sessionManager.apiError.connect(showErrorMsg);
    }
    Component.onDestruction: {
        extensionData.sessionManager.apiError.disconnect(showErrorMsg);
    }

    function showErrorMsg(msg) {
        errorMessage.text = msg;
    }

    Rectangle {
        id: body
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.right: parent.right

        anchors.leftMargin: 8
        // anchors.topMargin: 8
        anchors.rightMargin: 8

        height: childrenRect.height + 32
        radius: 8
        color: Qt.rgba(255, 255, 255, 0.45)

        Column {
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.right: parent.right
            anchors.leftMargin: 16
            anchors.topMargin: 16
            anchors.rightMargin: 16

            height: childrenRect.height
            spacing: 16

            Text {
                width: parent.width
                height: 40
                verticalAlignment: Text.AlignVCenter
                text: qsTr("Ai Assistant")
                font.bold: true
                font.pixelSize: 18
            }

            Text {
                width: parent.width
                height: contentHeight
                text: qsTr("AI assistant is a large language model of artificial intelligence, which has the ability of deep semantic understanding and generation across modes and languages. It has five abilities: literary creation, commercial copy creation, mathematical logic calculation, Chinese understanding, multimodal generation, and so on. It has a broader imagination space in many fields, such as search question and answer, content creation, intelligent office and so on.")
                wrapMode: Text.WordWrap
                color: Qt.rgba(0, 0, 0, 0.55)
            }

            Rectangle {
                width: parent.width
                height: 1

                color: Qt.rgba(38, 38, 38, 0.2)
            }

            Input {
                id: keyInput
                width: parent.width
                height: 40
                radius: 8
                placeholder: qsTr("Input API Key")
                color: Qt.rgba(38, 38, 38, 0.45)
            }

            Input {
                id: secretInput
                width: parent.width
                height: 40
                radius: 8
                placeholder: qsTr("Input Secret Key")
                color: Qt.rgba(38, 38, 38, 0.45)
            }

            Rectangle {
                width: parent.width
                height: 40
                radius: 8
                color: "#3790FA"

                Text {
                    anchors.fill: parent
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    text: qsTr("Confirm")
                    color: "#ffffff"
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        extensionData.sessionManager.setSessionKeys(keyInput.text, secretInput.text);
                    }
                }
            }
        }
    }

    Text {
        id: errorMessage
        anchors.top: body.bottom
        anchors.topMargin: 16
        width: parent.width
        height: contentHeight
        wrapMode: Text.WordWrap
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        color: "#E41A2B"
    }
}
