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

    property bool disableInput: false
    property alias text: messageInput.text
    property alias placeholder: tipText.text
    readonly property alias textInput: messageInput

    Item {
        anchors.fill: parent
        anchors.leftMargin: 8
        anchors.rightMargin: 8
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
        }

        Text {
            id: tipText
            anchors.fill: parent

            visible: messageInput.length === 0
            verticalAlignment: Text.AlignVCenter
            color: Qt.rgba(0, 0, 0, 0.3)
        }
    }
}
