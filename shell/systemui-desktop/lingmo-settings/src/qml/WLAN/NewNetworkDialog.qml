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

import QtQuick 2.4
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.0

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
    modality: Qt.WindowModal

    property int contentWidth: _mainLayout.implicitWidth + header.height + LingmoUI.Units.largeSpacing * 2
    property int contentHeight: _mainLayout.implicitHeight + header.height + LingmoUI.Units.largeSpacing * 2

    visible: false
    minimizeButtonVisible: false

    background.color: LingmoUI.Theme.secondBackgroundColor
    flags: Qt.Dialog | Qt.FramelessWindowHint

    signal connect(var ssid, var username, var pwd, var type)

    onVisibleChanged: {
        if (visible) {
            _userNameTextField.forceActiveFocus()
        } else {
            _nameTextField.clear()
            _userNameTextField.clear()
            _passwordTextField.clear()
            securitybox.currentIndex = 2
        }
    }

    ColumnLayout {
        id: _mainLayout
        anchors.fill: parent
        anchors.margins: LingmoUI.Units.largeSpacing
        anchors.topMargin: 0
        spacing: LingmoUI.Units.largeSpacing

        GridLayout {
            columns: 2
            columnSpacing: LingmoUI.Units.largeSpacing * 2
            rowSpacing: LingmoUI.Units.largeSpacing

            Label {
                text: qsTr("Name")
            }

            TextField {
                id: _nameTextField
                placeholderText: qsTr("Network Name")
                Layout.fillWidth: true
                Keys.onEscapePressed: control.close()
            }

            Label {
                text: qsTr("Security")
            }

            ComboBox {
                id: securitybox
                Layout.fillWidth: true
                currentIndex: 2
                model: [ qsTr("None"), "WEP", "WPA/WPA2", "WPA3" ]
            }

            Label {
                visible: securitybox.currentIndex === 1
                text: qsTr("UserName")
            }

            TextField {
                id: _userNameTextField
                visible: securitybox.currentIndex === 1
                placeholderText: qsTr("Password")
                focus: true
                Layout.fillWidth: true
                Keys.onEscapePressed: control.close()
            }

            Label {
                visible: securitybox.currentIndex != 0
                text: qsTr("Password")
            }

            TextField {
                visible: securitybox.currentIndex != 0
                id: _passwordTextField
                placeholderText: qsTr("Password")
                echoMode: TextInput.Password
                Layout.fillWidth: true
                Keys.onEscapePressed: control.close()
            }
        }

        RowLayout {
            spacing: LingmoUI.Units.largeSpacing

            Button {
                text: qsTr("Cancel")
                Layout.fillWidth: true
                onClicked: control.close()
            }

            Button {
                enabled: _nameTextField.text != ""
                         & (_passwordTextField.text.length > 7 | securitybox.currentIndex === 0)
                flat: true
                text: qsTr("Join")
                Layout.fillWidth: true
                onClicked: {
                    var security_type = securitybox.currentText

                    if (security_type.currentIndex === 0)
                        type = "None"

                    control.connect(_nameTextField.text,
                                    _userNameTextField.text,
                                    _passwordTextField.text,
                                    security_type)

                    control.close()
                }
            }
        }
    }
}
