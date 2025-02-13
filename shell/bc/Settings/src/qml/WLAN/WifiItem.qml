/*
 * Copyright (C) 2021 LingmoOS Team.
 *
 * Author:     revenmartin <revenmartin@gmail.com>
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
import QtQuick.Window 2.3

import LingmoUI 1.0 as LingmoUI
import Lingmo.NetworkManagement 1.0 as NM
import "../"

Item {
    id: control

    height: _itemLayout.implicitHeight + LingmoUI.Units.largeSpacing

    property bool passwordIsStatic: (model.securityType === NM.Enums.StaticWep || model.securityType === NM.Enums.WpaPsk ||
                                     model.securityType === NM.Enums.Wpa2Psk || model.securityType === NM.Enums.SAE)
    property bool predictableWirelessPassword: !model.uuid && model.type === NM.Enums.Wireless && passwordIsStatic

    ColumnLayout {
        id: _itemLayout
        anchors.fill: parent
        anchors.leftMargin: 0
        anchors.rightMargin: 0
        anchors.topMargin: LingmoUI.Units.smallSpacing
        anchors.bottomMargin: LingmoUI.Units.smallSpacing
        spacing: 0

        // 顶部项
        Item {
            Layout.fillWidth: true
            Layout.preferredHeight: _topItem.implicitHeight + LingmoUI.Units.largeSpacing

            Rectangle {
                anchors.fill: parent
                radius: LingmoUI.Theme.smallRadius
                color: LingmoUI.Theme.textColor
                opacity: mouseArea.pressed ? 0.15 :  mouseArea.containsMouse ? 0.1 : 0.0
            }

            MouseArea {
                id: mouseArea
                anchors.fill: parent
                hoverEnabled: true
                acceptedButtons: Qt.LeftButton

                onClicked: {
                    if (busyIndicator.visible)
                        return

                    if (model.uuid || !predictableWirelessPassword) {
                        if (connectionState === NM.Enums.Deactivated) {
                            if (!predictableWirelessPassword && !model.uuid) {
                                handler.addAndActivateConnection(model.devicePath, model.specificPath)
                            } else {
                                handler.activateConnection(model.connectionPath, model.devicePath, model.specificPath)
                            }
                        } else {
                            additionalSettings.toggle()
                        }
                    } else if (predictableWirelessPassword) {
                        connectDialog.name = model.itemUniqueName
                        connectDialog.securityType = model.securityType
                        connectDialog.devicePath = model.devicePath
                        connectDialog.specificPath = model.specificPath
                        connectDialog.show()
                    }

//                    if (model.uuid || !predictableWirelessPassword) {
//                        if (connectionState === NM.Enums.Deactivated) {
//                            if (!predictableWirelessPassword && !model.uuid) {
//                                handler.addAndActivateConnection(model.devicePath, model.specificPath);
//                            } else {
//                                handler.activateConnection(model.connectionPath, model.devicePath, model.specificPath);
//                            }
//                        } else {
//                            additionalSettings.toggle()
//                            // handler.deactivateConnection(model.connectionPath, model.devicePath);
//                        }
//                    } else if (predictableWirelessPassword) {
//                        // passwordDialog.show()

//                        // wirelessView.hideAllItems()
//                        additionalSettings.toggle()
//                    }
                }
            }

            RowLayout {
                id: _topItem
                anchors.fill: parent
                anchors.leftMargin: LingmoUI.Units.smallSpacing
                anchors.rightMargin: LingmoUI.Units.smallSpacing
                spacing: LingmoUI.Units.largeSpacing

                Image {
                    width: 22
                    height: width
                    sourceSize: Qt.size(width, height)
                    source: "qrc:/images/" + (LingmoUI.Theme.darkMode ? "dark/" : "light/") + model.connectionIcon + ".svg"
                    smooth: false
                }

                Label {
                    text: model.itemUniqueName
                }

                Item {
                    Layout.fillWidth: true
                }

                LingmoUI.BusyIndicator {
                    id: busyIndicator
                    width: 22
                    height: width
                    visible: connectionState === NM.Enums.Activating ||
                             connectionState === NM.Enums.Deactivating
                    running: busyIndicator.visible
                }

                // Activated
                Image {
                    width: 16
                    height: width
                    sourceSize: Qt.size(width, height)
                    source: "qrc:/images/light/checked.svg"
                    visible: model.connectionState === NM.Enums.Activated

                    ColorOverlay {
                        anchors.fill: parent
                        source: parent
                        color: LingmoUI.Theme.highlightColor
                        opacity: 1
                        visible: true
                    }
                }

                // Locked
                Image {
                    width: 22
                    height: width
                    sourceSize: Qt.size(width, height)
                    source: LingmoUI.Theme.darkMode ? "qrc:/images/dark/locked.svg" : "qrc:/images/light/locked.svg"
                    visible: (model.securityType === -1 | model.securityType === 0) ? false : true
                    smooth: false
                }

                IconButton {
                    source: "qrc:/images/info.svg"
                    onClicked: {
                        var component = Qt.createComponent("WirelessDetailsDialog.qml")
                        if (component.status === Component.Ready) {
                            var dialog = component.createObject(rootWindow)
                            dialog.open()
                        }
                    }
                }
            }
        }

        Hideable {
            id: additionalSettings
            spacing: 0

            onShownChanged: {
                // 清除数据
                if (!shown) {
                    passwordField.clear()
                } else {
                    // wirelessView.contentY = control.y
                    passwordField.forceActiveFocus()
                }
            }

            Item {
                height: LingmoUI.Units.largeSpacing * 2
            }

            // 密码对话
            RowLayout {
                visible: predictableWirelessPassword
                spacing: LingmoUI.Units.largeSpacing

                Label {
                    text: qsTr("Password")
                }

                TextField {
                    id: passwordField
                    focus: true
                    echoMode: TextInput.Password
                    selectByMouse: true
                    placeholderText: qsTr("Password")
                    validator: RegExpValidator {
                        regExp: {
                            if (model.securityType === NM.Enums.StaticWep)
                                return /^(?:[\x20-\x7F]{5}|[0-9a-fA-F]{10}|[\x20-\x7F]{13}|[0-9a-fA-F]{26}){1}$/;
                            return /^(?:[\x20-\x7F]{8,64}){1}$/;
                        }
                    }
                    onAccepted: connectWithPassword()
                    Keys.onEscapePressed: additionalSettings.toggle()
                    Layout.fillWidth: true
                }

                Button {
                    text: qsTr("Cancel")
                    onClicked: additionalSettings.hide()
                }

                Button {
                    text: qsTr("Connect")
                    flat: true
                    enabled: passwordField.acceptableInput
                    onClicked: connectWithPassword()
                }
            }

            RowLayout {
                visible: !predictableWirelessPassword

                Button {
                    visible: (model.uuid || !predictableWirelessPassword) && connectionState === NM.Enums.Deactivated
                    text: qsTr("Connect")
                    flat: true

                    onClicked: {
                        if (!predictableWirelessPassword && !model.uuid) {
                            handler.addAndActivateConnection(model.devicePath, model.specificPath)
                        } else {
                            handler.activateConnection(model.connectionPath, model.devicePath, model.specificPath)
                        }
                    }
                }

                Button {
                    visible: connectionState === NM.Enums.Activated
                    text: qsTr("Disconnect")

                    onClicked: {
                        handler.deactivateConnection(model.connectionPath, model.devicePath)
                    }
                }
            }

            Item {
                height: LingmoUI.Units.smallSpacing
            }

            HorizontalDivider {}
        }
    }

    function connectWithPassword() {
        handler.addAndActivateConnection(model.devicePath, model.specificPath, passwordField.text)
    }

    function hideAdditional() {
        additionalSettings.hide()
    }
}
