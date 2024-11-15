/*
 * Copyright (C) 2021 LingmoOS Team.
 *
 * Author:     Kate Leet <kate@lingmoos.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import LingmoUI 1.0 as LingmoUI
import Lingmo.NetworkManagement 1.0 as NM

LingmoUI.Window {
    id: control

    width: contentWidth
    height: contentHeight
    minimumWidth: contentWidth
    minimumHeight: contentHeight
    maximumWidth: contentWidth
    maximumHeight: contentHeight

    property int contentWidth: _mainLayout.implicitWidth + header.height + LingmoUI.Units.largeSpacing * 2
    property int contentHeight: _mainLayout.implicitHeight + header.height + LingmoUI.Units.largeSpacing * 2

    visible: false
    minimizeButtonVisible: false
    modality: Qt.WindowModal

    background.color: LingmoUI.Theme.secondBackgroundColor
    flags: Qt.Dialog | Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint

    signal connect(var devicePath, var specificPath, var password)

    property var name: ""
    property var devicePath: ""
    property var specificPath: ""
    property var securityType: ""

    onVisibleChanged: {
        if (visible) {
            passwordField.clear()
            passwordField.forceActiveFocus()
        }
    }

    headerItem: Item {
        Label {
            anchors.centerIn: parent
            text: qsTr("Enter Password")
        }
    }

    ColumnLayout {
        id: _mainLayout
        anchors.fill: parent
        anchors.margins: LingmoUI.Units.largeSpacing
        anchors.topMargin: 0
        spacing: LingmoUI.Units.largeSpacing

        Label {
            text: qsTr("Enter the password for %1").arg(control.name)
            color: LingmoUI.Theme.disabledTextColor
            wrapMode: Text.WordWrap
        }

        TextField {
            id: passwordField
            focus: true
            echoMode: TextInput.Password
            selectByMouse: true
            placeholderText: qsTr("Password")

            validator: RegExpValidator {
                regExp: {
                    if (control.securityType === NM.Enums.StaticWep)
                        return /^(?:[\x20-\x7F]{5}|[0-9a-fA-F]{10}|[\x20-\x7F]{13}|[0-9a-fA-F]{26}){1}$/;
                    return /^(?:[\x20-\x7F]{8,64}){1}$/;
                }
            }

            onAccepted: {
                control.emitSignal()
            }

            Keys.onEscapePressed: {
                control.visible = false
            }

            Layout.fillWidth: true
        }

        RowLayout {
            spacing: LingmoUI.Units.largeSpacing

            Button {
                text: qsTr("Cancel")
                Layout.fillWidth: true
                onClicked: control.visible = false
            }

            Button {
                text: qsTr("Connect")
                flat: true
                Layout.fillWidth: true
                enabled: passwordField.acceptableInput
                onClicked: control.emitSignal()
            }
        }
    }

    function emitSignal() {
        control.connect(control.devicePath, control.specificPath, passwordField.text)
        control.visible = false
    }
}
