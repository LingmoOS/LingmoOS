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

import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtQuick.Dialogs 1.2
import QtGraphicalEffects 1.0

import Lingmo.Settings 1.0
import Lingmo.Accounts 1.0
import LingmoUI 1.0 as LingmoUI

import "../"

RoundedItem {
    id: control

    height: mainLayout.implicitHeight + LingmoUI.Units.largeSpacing * 2

    UserAccount {
        id: currentUser
        userId: model.userId
    }

    FileDialog {
        id: fileDialog
        folder: shortcuts.pictures
        nameFilters: ["Image files (*.jpg *.png)", "All files (*)"]
        onAccepted: {
            currentUser.iconFileName = fileDialog.fileUrl.toString().replace("file://", "")
            _userImage.source = fileDialog.fileUrl
            _userImage.update()
        }
    }

    ColumnLayout {
        id: mainLayout
        spacing: 0

        RowLayout {
            id: _itemLayout
            spacing: 0

            Item {
                id: _topItem

                Layout.fillWidth: true
                height: _topLayout.implicitHeight + LingmoUI.Units.largeSpacing

                MouseArea {
                    anchors.fill: parent
                    onDoubleClicked: additionalSettings.toggle()
                }

                RowLayout {
                    id: _topLayout
                    anchors.fill: parent
                    anchors.topMargin: LingmoUI.Units.smallSpacing
                    anchors.bottomMargin: LingmoUI.Units.smallSpacing
                    spacing: 0

                    Image {
                        id: _userImage

                        property int iconSize: 48

                        Layout.preferredWidth: iconSize
                        Layout.preferredHeight: iconSize
                        sourceSize: String(source) === "image://icontheme/default-user" ? Qt.size(iconSize, iconSize) : undefined
                        source: iconFileName ? "file:///" + iconFileName : "image://icontheme/default-user"
                        visible: status === Image.Ready
                        Layout.alignment: Qt.AlignVCenter

                        MouseArea {
                            anchors.fill: parent
                            onClicked: fileDialog.open()
                            cursorShape: Qt.PointingHandCursor
                        }

                        layer.enabled: true
                        layer.effect: OpacityMask {
                            maskSource: Item {
                                width: _userImage.width
                                height: width

                                Rectangle {
                                    anchors.fill: parent
                                    radius: width / 2
                                }
                            }
                        }
                    }

                    Label {
                        Layout.alignment: Qt.AlignVCenter
                        font.pixelSize: 15
                        text: "<b>%1</b>".arg(userName)
                        leftPadding: LingmoUI.Units.largeSpacing
                    }

                    Item {
                        width: LingmoUI.Units.largeSpacing
                    }

                    Label {
                        Layout.alignment: Qt.AlignVCenter
                        text: realName
                        color: LingmoUI.Theme.disabledTextColor
                        visible: realName !== userName
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                    Label {
                        text: qsTr("Currently logged")
                        Layout.alignment: Qt.AlignVCenter
                        visible: currentUser.userId === loggedUser.userId
                    }

                    Item {
                        width: LingmoUI.Units.smallSpacing
                    }

                    LingmoUI.RoundImageButton {
                        iconMargins: LingmoUI.Units.smallSpacing
                        source: LingmoUI.Theme.darkMode ? additionalSettings.shown ? "qrc:/images/dark/up.svg" : "qrc:/images/dark/down.svg"
                                                      : additionalSettings.shown ? "qrc:/images/light/up.svg" : "qrc:/images/light/down.svg"
                        onClicked: additionalSettings.toggle()
                        Layout.alignment: Qt.AlignVCenter
                    }
                }
            }
        }

        Hideable {
            id: additionalSettings
            spacing: LingmoUI.Units.largeSpacing

            Item {
                height: LingmoUI.Units.largeSpacing
            }

            GridLayout {
                Layout.fillWidth: true
                Layout.bottomMargin: LingmoUI.Units.smallSpacing
                rowSpacing: LingmoUI.Units.largeSpacing * 2
                columns: 2

                Label {
                    text: qsTr("Account type")
                }

                Label {
                    text: currentUser.accountType === 0 ? qsTr("Standard")
                                                        : qsTr("Administrator")
                }

                Label {
                    text: qsTr("Automatic login")
                    Layout.fillWidth: true
                }

                Switch {
                    id: automaticLoginSwitch
                    Layout.fillHeight: true
                    leftPadding: 0
                    rightPadding: 0
                    onCheckedChanged: currentUser.automaticLogin = checked
                    Layout.alignment: Qt.AlignVCenter | Qt.AlignRight

                    Component.onCompleted: {
                        automaticLoginSwitch.checked = currentUser.automaticLogin
                    }
                }
            }

            // Change password
            HorizontalDivider {
                visible: changePasswdLabel.visible
            }

            Label {
                id: changePasswdLabel
                visible: false
                text: qsTr("Change password")
            }

            GridLayout {
                id: changePasswdLayout
                visible: false
                columns: 2
                columnSpacing: LingmoUI.Units.largeSpacing * 2
                rowSpacing: LingmoUI.Units.smallSpacing * 2

                Label {
                    text: qsTr("Password")
                    Layout.alignment: Qt.AlignRight
                }

                TextField {
                    id: passwordField
                    placeholderText: qsTr("Password")
                    echoMode: TextField.Password
                    Layout.fillWidth: true
                    selectByMouse: true
                }

                Label {
                    text: qsTr("Verify password")
                    Layout.alignment: Qt.AlignRight
                }

                TextField {
                    id: verifyPasswordField
                    placeholderText: qsTr("Verify password")
                    echoMode: TextField.Password
                    Layout.fillWidth: true
                    selectByMouse: true
                }
            }

            RowLayout {
                id: changePasswdFooterLayout
                spacing: LingmoUI.Units.largeSpacing
                visible: false

                Button {
                    text: qsTr("Cancel")
                    onClicked: hideChangePasswordItem()
                    Layout.fillWidth: true
                }

                Button {
                    text: qsTr("Change password")
                    enabled: passwordField.text != "" &&
                             passwordField.text == verifyPasswordField.text
                    Layout.fillWidth: true
                    flat: true
                    onClicked: {
                        currentUser.setPassword(Password.cryptPassword(passwordField.text));
                        hideChangePasswordItem()
                    }
                }
            }

            HorizontalDivider {
                visible: changePasswdLabel.visible
            }

            // Change password end.

            StandardButton {
                text: qsTr("Change password")
                onClicked: showChangePasswordItem()
                backgroundColor: LingmoUI.Theme.darkMode ? "#363636" : LingmoUI.Theme.backgroundColor
                Layout.fillWidth: true
                visible: !changePasswdLabel.visible
            }

            StandardButton {
                text: qsTr("Delete this user")
                enabled: model.userId !== loggedUser.userId
                onClicked: accountsManager.deleteUser(userId, true)
                backgroundColor: LingmoUI.Theme.darkMode ? "#363636" : LingmoUI.Theme.backgroundColor
                Layout.fillWidth: true
            }
        }
    }

    function showChangePasswordItem() {
        changePasswdLabel.visible = true
        changePasswdLayout.visible = true
        changePasswdFooterLayout.visible = true

        passwordField.forceActiveFocus()
    }

    function hideChangePasswordItem() {
        passwordField.clear()
        verifyPasswordField.clear()

        changePasswdLabel.visible = false
        changePasswdLayout.visible = false
        changePasswdFooterLayout.visible = false
    }
}
