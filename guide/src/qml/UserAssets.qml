/*
 * Copyright (C) 2024 LingmoOS Team.
 *
 * Author:     LingmoOS Team <team@lingmo.org>
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
import QtGraphicalEffects 1.0
import QtQuick.Layouts 1.12
import LingmoUI 1.0 as LingmoUI
import Lingmo.Setup 1.0

Item {
    id: control

    ColumnLayout {
        id: layout
        anchors.fill: parent

        Item {
            height: LingmoUI.Units.largeSpacing
        }

        RoundedItem {
            id: user

            function clear() {
                userNameField.clear()
                passwordField.clear()
                verifyPasswordField.clear()
                accountTypeCombo.currentIndex = 0
            }

            Image {
                id: userImag
                width: 350
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.leftMargin: user.width/20
                sourceSize: Qt.size(width, height)
                source: "qrc:/assets/user_set.svg"
            }

            Text {
                id: hello
                anchors.left: userImag.right
                anchors.leftMargin: userImag.width/8
                anchors.top: parent.top
                anchors.topMargin: 30
                text: qsTr("Create your own account")
                font.pointSize: 20
                font.bold: true
                color: LingmoUI.Theme.textColor
            }

            Text {
                id: txt
                anchors.left: userImag.right
                anchors.top: hello.bottom
                anchors.topMargin: 5
                anchors.leftMargin: userImag.width/8
                text: qsTr("We will create a new account on this computer for your daily use, in addition to the root account. \nPlease make sure to fill in the relevant information.")
                wrapMode: txt.WordWrap
                color: LingmoUI.Theme.textColor
                font.pointSize: 10
            }

            Text {
                id: userName
                anchors.left: userImag.right
                anchors.top: txt.bottom
                anchors.topMargin: 25
                anchors.leftMargin: userImag.width/8
                text: qsTr("User Name")
                color: LingmoUI.Theme.textColor
                font.pointSize: 15
            }

            TextField {
                id: userNameField
                anchors.left: userImag.right
                anchors.top: userName.bottom
                anchors.topMargin: 10
                anchors.leftMargin: userImag.width/8
                width: parent.width/2.5
                placeholderText: qsTr("User name")
                selectByMouse: true
            }

            Text {
                id: passWd
                anchors.left: userImag.right
                anchors.top: userNameField.bottom
                anchors.topMargin: 18
                anchors.leftMargin: userImag.width/8
                text: qsTr("Password")
                color: LingmoUI.Theme.textColor
                font.pointSize: 15
            }

            TextField {
                id: passwordField
                anchors.left: userImag.right
                anchors.top: passWd.bottom
                anchors.topMargin: 10
                anchors.leftMargin: userImag.width/8
                width: parent.width/2.5
                placeholderText: qsTr("Password")
                echoMode: TextField.Password
                selectByMouse: true
            }

            TextField {
                id: verifyPasswordField
                anchors.left: userImag.right
                anchors.top: passwordField.bottom
                anchors.topMargin: 8
                anchors.leftMargin: userImag.width/8
                width: parent.width/2.5
                placeholderText: qsTr("Please re-enter your password")
                echoMode: TextField.Password
                selectByMouse: true
            }

            Text {
                id: pcName
                anchors.left: userImag.right
                anchors.top: verifyPasswordField.bottom
                anchors.topMargin: 18
                anchors.leftMargin: userImag.width/8
                text: qsTr("PC Name")
                color: LingmoUI.Theme.textColor
                font.pointSize: 15
            }

            TextField {
                id: pcNameField
                anchors.left: userImag.right
                anchors.top: pcName.bottom
                anchors.topMargin: 10
                anchors.leftMargin: userImag.width/8
                width: parent.width/2.5
                placeholderText: qsTr("Your PC Name")
                // echoMode: TextField.Password
                selectByMouse: true
            }

            Button {
                anchors.right: parent.right
                anchors.rightMargin: parent.width/20
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 35
                flat: true
                text: qsTr("Next")
                font.pointSize: 13
                onClicked: {
                    stackView.push(secondPage)
                }
            }
        }
    }
}