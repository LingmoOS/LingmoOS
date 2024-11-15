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
import QtGraphicalEffects 1.0

import LingmoUI 1.0 as LingmoUI
import Lingmo.Settings 1.0
import Lingmo.Accounts 1.0
import "../"

ItemPage {
    headerTitle: qsTr("User")

    UserAccount {
        id: loggedUser
    }

    UsersModel {
        id: userModel
    }

    AccountsManager {
        id: accountsManager

        onUserAdded: {
            if (account.userName === userNameField.text) {
                account.passwordMode = UserAccount.RegularPasswordMode
                account.setPassword(Password.cryptPassword(passwordField.text))
                newUserItem.clear()
            }
        }
    }

    Scrollable {
        id: _scrollable
        anchors.fill: parent
        contentHeight: layout.implicitHeight

        ColumnLayout {
            id: layout
            anchors.fill: parent
            spacing: LingmoUI.Units.largeSpacing * 2

            ListView {
                id: _userView
                model: userModel
                Layout.fillWidth: true
                spacing: LingmoUI.Units.largeSpacing * 2
                interactive: false

                Layout.preferredHeight: {
                    var totalHeight = 0
                    for (var i = 0; i < _userView.visibleChildren.length; ++i) {
                        totalHeight += _userView.visibleChildren[i].height
                    }
                    return totalHeight
                }

                delegate: UserDelegateItem {
                    width: _userView.width
                }
            }

            // Add new user.
            RoundedItem {
                id: newUserItem

                visible: false

                function clear() {
                    userNameField.clear()
                    passwordField.clear()
                    verifyPasswordField.clear()
                    accountTypeCombo.currentIndex = 0
                }

                ColumnLayout {
                    id: _mainLayout
                    spacing: LingmoUI.Units.largeSpacing * 1.5

                    Label {
                        text: qsTr("Add new user")
                    }

                    GridLayout {
                        columns: 2
                        columnSpacing: LingmoUI.Units.largeSpacing * 2
                        rowSpacing: LingmoUI.Units.smallSpacing * 2

                        Label {
                            text: qsTr("User name")
                            Layout.alignment: Qt.AlignRight
                        }

                        TextField {
                            id: userNameField
                            placeholderText: qsTr("User name")
                            Layout.fillWidth: true
                            Layout.alignment: Qt.AlignRight
                            selectByMouse: true
                        }

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

                        Label {
                            text: qsTr("Account type")
                            Layout.alignment: Qt.AlignRight
                        }

                        ComboBox {
                            id: accountTypeCombo
                            model: [qsTr("Standard"), qsTr("Administrator")]
                            Layout.fillWidth: true
                            topInset: 0
                            bottomInset: 0
                        }
                    }

                    RowLayout {
                        id: footerLayout
                        spacing: LingmoUI.Units.largeSpacing

                        Button {
                            id: cancelButton
                            text: qsTr("Cancel")
                            onClicked: {
                                newUserItem.visible = false
                                newUserItem.clear()
                            }
                            Layout.fillWidth: true
                        }

                        Button {
                            id: addButton
                            text: qsTr("Add")
                            enabled: userNameField.text != "" &&
                                     passwordField.text != "" &&
                                     passwordField.text == verifyPasswordField.text
                            Layout.fillWidth: true
                            flat: true
                            onClicked: {
                                if (accountsManager.createUser(userNameField.text, "", accountTypeCombo.currentIndex)) {
                                    newUserItem.visible = true
                                }
                            }
                        }
                    }
                }
            }

            StandardButton {
                id: _addUserButton
                text: ""
                Layout.fillWidth: true
                onClicked: {
                    newUserItem.visible = true
                    userNameField.forceActiveFocus()
                    _scrollable.contentY = Math.max(newUserItem.y,
                                                    _scrollable.contentHeight)
                }

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: LingmoUI.Units.largeSpacing * 1.5
                    anchors.rightMargin: LingmoUI.Units.largeSpacing * 1.5

                    Label {
                        text: qsTr("Add user")
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                    Label {
                        text: qsTr("→")
                    }
                }
            }

            Item {
                height: LingmoUI.Units.largeSpacing
            }
        }
    }
}
