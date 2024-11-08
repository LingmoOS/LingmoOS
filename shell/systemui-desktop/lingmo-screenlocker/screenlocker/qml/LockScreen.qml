/*
 * Copyright (C) 2021 CuteOS Team.
 *
 * Author:     Rion Wong <reionwong@gmail.com>
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
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtGraphicalEffects 1.0

import Lingmo.Accounts 1.0 as Accounts
import Lingmo.System 1.0 as System
import LingmoUI 1.0 as CuteUI

Item {
    id: root

    property string notification

    LayoutMirroring.enabled: Qt.locale().textDirection === Qt.RightToLeft
    LayoutMirroring.childrenInherit: true

    System.Wallpaper {
        id: wallpaper
    }

    Image {
        id: wallpaperImage
        anchors.fill: parent
        source: "file://" + wallpaper.path
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
        radius: 0
        source: wallpaperImage
        cached: true
        visible: true
    }

    NumberAnimation {
        id: blurAni
        target: wallpaperBlur
        property: "radius"
        duration: 300
        from: 10
        to: 64
    }

    Accounts.UserAccount {
        id: currentUser
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

        blurAni.start()
    }

    Item {
        id: _topItem
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: _mainItem.top
        anchors.bottomMargin: root.height * 0.1

//        Image {
//            id: icon
//            anchors.horizontalCenter: parent.horizontalCenter
//            anchors.verticalCenter: parent.verticalCenter
//            Layout.alignment: Qt.AlignHCenter
//            width: 32
//            height: 32
//            sourceSize: Qt.size(width, height)
//            source: "qrc:/images/system-lock-screen-symbolic.svg"
//        }

        Label {
            id: timeLabel
//            anchors.top: icon.bottom
//            anchors.topMargin: CuteUI.Units.largeSpacing
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
            Layout.alignment: Qt.AlignHCenter
            font.pointSize: 35
            color: "white"

            function updateInfo() {
                timeLabel.text = new Date().toLocaleString(Qt.locale(), "hh:mm")
            }
        }

        Label {
            id: dateLabel
            anchors.top: timeLabel.bottom
            anchors.topMargin: CuteUI.Units.largeSpacing
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
        width: 280 + CuteUI.Units.largeSpacing * 3
        height: _mainLayout.implicitHeight + CuteUI.Units.largeSpacing * 4

        Layout.alignment: Qt.AlignHCenter

        Rectangle {
            anchors.fill: parent
            radius: CuteUI.Theme.bigRadius + 2
            color: CuteUI.Theme.darkMode ? "#424242" : "white"
            opacity: 0.5
        }

        ColumnLayout {
            id: _mainLayout
            anchors.fill: parent
            anchors.margins: CuteUI.Units.largeSpacing * 1.5
            spacing: CuteUI.Units.smallSpacing * 1.5

            Image {
                id: userIcon

                property int iconSize: 60

                Layout.preferredHeight: iconSize
                Layout.preferredWidth: iconSize
                sourceSize: String(source) === "image://icontheme/default-user" ? Qt.size(iconSize, iconSize) : undefined
                source: currentUser.iconFileName ? "file:///" + currentUser.iconFileName : "image://icontheme/default-user"
                Layout.alignment: Qt.AlignHCenter

                layer.enabled: true
                layer.effect: OpacityMask {
                    maskSource: Item {
                        width: userIcon.width
                        height: userIcon.height

                        Rectangle {
                            anchors.fill: parent
                            radius: parent.height / 2
                        }
                    }
                }
            }

            Label {
                Layout.alignment: Qt.AlignHCenter
                text: currentUser.userName
            }

            Item {
                height: 1
            }

            TextField {
                id: password
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredHeight: 36
                Layout.fillWidth: true
                placeholderText: qsTr("Password")
                leftPadding: CuteUI.Units.largeSpacing
                rightPadding: 36 + CuteUI.Units.largeSpacing //CuteUI.Units.largeSpacing
                enabled: !authenticator.graceLocked
                focus: true

                echoMode: TextInput.Password

                background: Rectangle {
                    color: CuteUI.Theme.darkMode ? "#B6B6B6" : "white"
                    // radius: CuteUI.Theme.bigRadius
                    opacity: 0.5
                }

                Keys.onEnterPressed: root.tryUnlock()
                Keys.onReturnPressed: root.tryUnlock()
                Keys.onEscapePressed: password.text = ""

                LoginButton {
                    anchors.right: password.right
                    anchors.top: password.top
                    anchors.bottom: password.bottom
                    source: "qrc:/images/screensaver-unlock-symbolic.svg"
                    iconMargins: 10
                    Layout.alignment: Qt.AlignHCenter
                    enabled: !authenticator.graceLocked
                    onClicked: root.tryUnlock()
                    size: 36
                    Layout.preferredHeight: 36
                    Layout.preferredWidth: 36
                }

                layer.enabled: true
                layer.effect: OpacityMask {
                    maskSource: Item {
                        width: password.width
                        height: password.height

                        Rectangle {
                            anchors.fill: parent
                            radius: CuteUI.Theme.mediumRadius
                        }
                    }
                }
            }

            Item {
                height: 1
            }
        }
    }

    Label {
        id: message
        anchors.top: _mainItem.bottom
        anchors.topMargin: CuteUI.Units.largeSpacing
        anchors.horizontalCenter: parent.horizontalCenter
        font.bold: true
        color: "white"
        text: root.notification

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
        opacity: 0.3
        visible: true
    }

    Item {
        // anchors.top: message.bottom
        // anchors.topMargin: CuteUI.Units.largeSpacing
        anchors.bottom: parent.bottom
        anchors.bottomMargin: root.height * 0.05 //CuteUI.Units.largeSpacing
        anchors.horizontalCenter: parent.horizontalCenter

        width: 280 + CuteUI.Units.largeSpacing * 3
        height: 70

        MprisItem {
            anchors.fill: parent
        }
    }

    function tryUnlock() {
        if (!password.text) {
            notificationResetTimer.start()
            root.notification = qsTr("Please enter your password")
            return
        }

        authenticator.tryUnlock(password.text)
    }

    Timer {
        id: notificationResetTimer
        interval: 3000
        onTriggered: root.notification = ""
    }

    Connections {
        target: authenticator

        function onFailed() {
            notificationResetTimer.start()
            root.notification = qsTr("Unlocking failed")
        }

        function onGraceLockedChanged() {
            if (!authenticator.graceLocked) {
                root.notification = ""
                password.selectAll()
                password.focus = true
            }
        }

        function onMessage(text) {
            notificationResetTimer.start()
            root.notification = text
        }

        function onError(text) {
            notificationResetTimer.start()
            root.notification = text
        }
    }
}
