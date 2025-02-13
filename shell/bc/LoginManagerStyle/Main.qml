/*
 * Copyright (C) 2024 LingmoOS.
 *
 * Author:     Reion Wong <reionwong@gmail.com>
 * Maintainer: Lingmo OS Team <team@lingmo.org>
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
import QtQuick.Window 2.12
import QtQuick.Controls 2.12 as QQC2
import QtQuick.Layouts 1.12
import QtGraphicalEffects 1.0

import Lingmo.Accounts 1.0 as Accounts
import Lingmo.System 1.0 as System
import LingmoUI 1.0 as LingmoUI

import SddmComponents 2.0
import "./"

Item {
    id: root

    property string notificationMessage

    LayoutMirroring.enabled: Qt.locale().textDirection == Qt.RightToLeft
    LayoutMirroring.childrenInherit: true

    TextConstants { id: textConstants }

    Image {
        id: wallpaperImage
        anchors.fill: parent
        // TODO: dynamically change to wallpaper for user
        source: "file://" + "/usr/share/backgrounds/lingmoos/default.jpg"
        sourceSize: Qt.size(width * Screen.devicePixelRatio,
                            height * Screen.devicePixelRatio)
        fillMode: Image.PreserveAspectCrop
        asynchronous: false
        clip: true
        cache: false
        smooth: true
    }

    FastBlur {
        id: wallpaperBlur
        anchors.fill: parent
        radius: 64
        source: wallpaperImage
        cached: true
        visible: true
    }

    Timer {
        repeat: true
        running: true
        interval: 1000

        onTriggered: {
            timeLabel.updateInfo()
            dateLabel.updateInfo()
        }
    }

    Component.onCompleted: {
        timeLabel.updateInfo()
        dateLabel.updateInfo()
    }

    Item {
        id: _topItem
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: _mainItem.top
        anchors.bottomMargin: root.height * 0.1

        QQC2.Label {
            id: timeLabel
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
            Layout.alignment: Qt.AlignHCenter
            font.pointSize: 35
            color: "white"

            function updateInfo() {
                timeLabel.text = new Date().toLocaleString(Qt.locale(), "hh:mm")
            }
        }

        QQC2.Label {
            id: dateLabel
            anchors.top: timeLabel.bottom
            anchors.topMargin: LingmoUI.Units.largeSpacing
            anchors.horizontalCenter: parent.horizontalCenter
            font.pointSize: 19
            color: "white"

            function updateInfo() {
                dateLabel.text = new Date().toLocaleDateString(Qt.locale(), Locale.LongFormat)
            }
        }

        DropShadow {
            anchors.fill: timeLabel
            source: timeLabel
            z: -1
            horizontalOffset: 1
            verticalOffset: 1
            radius: 10
            samples: radius * 4
            spread: 0.35
            color: Qt.rgba(0, 0, 0, 0.8)
            opacity: 0.1
            visible: true
        }

        DropShadow {
            anchors.fill: dateLabel
            source: dateLabel
            z: -1
            horizontalOffset: 1
            verticalOffset: 1
            radius: 10
            samples: radius * 4
            spread: 0.35
            color: Qt.rgba(0, 0, 0, 0.8)
            opacity: 0.1
            visible: true
        }
    }

    Item {
        id: _mainItem
        anchors.centerIn: parent
        width: 260 + LingmoUI.Units.largeSpacing * 3
        height: _mainLayout.implicitHeight + LingmoUI.Units.largeSpacing * 4

        Layout.alignment: Qt.AlignHCenter

        Rectangle {
            anchors.fill: parent
            radius: LingmoUI.Theme.bigRadius + 2
            color: LingmoUI.Theme.darkMode ? "#424242" : "white"
            opacity: 0.5
        }

        ColumnLayout {
            id: _mainLayout
            anchors.fill: parent
            anchors.margins: LingmoUI.Units.largeSpacing * 1.5
            spacing: LingmoUI.Units.smallSpacing * 1.5

            UserView {
                id: _userView
                model: userModel
                Layout.fillWidth: true
            }

            QQC2.TextField {
                id: passwordField
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredHeight: 40
                Layout.preferredWidth: 260
                placeholderText: qsTr("Password")
                focus: true
                selectByMouse: true
                echoMode: TextInput.Password

                background: Rectangle {
                    color: LingmoUI.Theme.darkMode ? "#B6B6B6" : "white"
                    radius: LingmoUI.Theme.bigRadius
                    opacity: 0.5
                }

                Keys.onEnterPressed: root.startLogin()
                Keys.onReturnPressed: root.startLogin()
                Keys.onEscapePressed: passwordField.text = ""
            }

            QQC2.Button {
                id: unlockBtn
                hoverEnabled: true
                focusPolicy: Qt.NoFocus
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredHeight: 40
                Layout.preferredWidth: 260
                text: qsTr("Login")
                onClicked: root.startLogin()

                scale: unlockBtn.pressed ? 0.95 : 1.0

                Behavior on scale {
                    NumberAnimation {
                        duration: 100
                    }
                }

                background: Rectangle {
                    color: LingmoUI.Theme.darkMode ? "#B6B6B6" : "white"
                    opacity: unlockBtn.pressed ? 0.3 : unlockBtn.hovered ? 0.4 : 0.5
                    radius: LingmoUI.Theme.bigRadius
                }
            }
        }
    }

    Item {
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        anchors.leftMargin: LingmoUI.Units.largeSpacing
        anchors.bottomMargin: LingmoUI.Units.largeSpacing

        width: sessionMenu.implicitWidth + LingmoUI.Units.largeSpacing
        height: sessionMenu.implicitHeight

        SessionMenu {
            id: sessionMenu
            anchors.fill: parent
            rootFontSize: 10
            height: 30
            Layout.fillWidth: true
        }
    }

    Item {
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.rightMargin: LingmoUI.Units.largeSpacing
        anchors.bottomMargin: LingmoUI.Units.smallSpacing * 1.5

        width: 50
        height: 50 + LingmoUI.Units.largeSpacing


        LingmoUI.RoundImageButton {
            anchors.fill: parent
            width: 50
            height: 50 + LingmoUI.Units.largeSpacing

            size: 50
            source: "system-shutdown-symbolic.svg"
            iconMargins: LingmoUI.Units.largeSpacing

            // anchors.top: message.bottom
            // anchors.topMargin: LingmoUI.Units.largeSpacing
            // anchors.horizontalCenter: parent.horizontalCenter
            onClicked: actionMenu.popup()
        }
    }

    LingmoUIMenu {
        id: actionMenu

        QQC2.MenuItem {
            text: qsTr("Suspend")
            onClicked: sddm.suspend()
            visible: sddm.canSuspend
        }

        QQC2.MenuItem {
            text: qsTr("Reboot")
            onClicked: sddm.reboot()
            visible: sddm.canReboot
        }

        QQC2.MenuItem {
            text: qsTr("Shutdown")
            onClicked: sddm.powerOff()
            visible: sddm.canPowerOff
        }
    }

    // LingmoUI.RoundImageButton {
    //     width: 50
    //     height: 50

    //     size: 50
    //     source: "system-shutdown-symbolic.svg"
    //     iconMargins: 10

    //     anchors.top: message.bottom
    //     anchors.topMargin: LingmoUI.Units.largeSpacing
    //     anchors.horizontalCenter: parent.horizontalCenter
    // }

    QQC2.Label {
        id: message
        anchors.top: _mainItem.bottom
        anchors.topMargin: LingmoUI.Units.largeSpacing
        anchors.horizontalCenter: parent.horizontalCenter
        font.bold: true
        text: root.notificationMessage
        color: "white"

        Behavior on opacity {
            NumberAnimation {
                duration: 250
            }
        }

        opacity: text == "" ? 0 : 1
    }

    DropShadow {
        anchors.fill: message
        source: message
        z: -1
        horizontalOffset: 1
        verticalOffset: 1
        radius: 10
        samples: radius * 4
        spread: 0.35
        color: Qt.rgba(0, 0, 0, 0.8)
        opacity: 0.1
        visible: true
    }

    function startLogin() {
        var username = _userView.currentItem.userName
        var password = passwordField.text
        root.notificationMessage = ""
        sddm.login(username, password, sessionMenu.currentIndex)
    }

    Timer {
        id: notificationResetTimer
        interval: 3000
        onTriggered: root.notificationMessage = ""
    }

    Connections {
        target: sddm

        function onLoginFailed() {
            notificationMessage = textConstants.loginFailed
            notificationResetTimer.start();
        }
    }
}
