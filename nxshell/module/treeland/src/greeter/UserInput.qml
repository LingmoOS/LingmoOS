// Copyright (C) 2023 justforlxz <justforlxz@gmail.com>.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.lingmo.dtk 1.0 as D
import TreeLand.Greeter

Item {
    id: loginGroup
    width: 220
    height: 300

    property string normalHint: qsTr("Please enter password")

    function updateUser() {
        let currentUser = GreeterModel.userModel.get(GreeterModel.currentUser)
        username.text = currentUser.realName.length == 0 ? currentUser.name : currentUser.realName
        passwordField.text = ''
        avatar.fallbackSource = currentUser.icon
        passwordField.visible = !currentUser.noPassword
        updateHintMsg(normalHint)
    }

    function userLogin() {
        let user = GreeterModel.userModel.get(GreeterModel.currentUser)
        if (user.logined) {
            GreeterModel.proxy.unlock(user.name, passwordField.text)
            return
        }

        GreeterModel.proxy.login(user.name, passwordField.text,
                                 GreeterModel.currentSession)
    }

    Connections {
        target: GreeterModel.userModel
        function onUpdateTranslations() {
            updateUser()
        }
    }

    Connections {
        target: Greeter
        function onVisibleChanged(visible) {
            if (!visible) {
                passwordField.text = ''
            }
        }
    }

    Rectangle {
        anchors.horizontalCenter: parent.horizontalCenter
        width: 32
        height: 32
        color: "transparent"
        anchors.bottom: parent.top
        anchors.bottomMargin: 56
        D.DciIcon {
            name: "login_lock"
            anchors.centerIn: parent
            sourceSize {
                width: 18
                height: 22
            }
        }
    }

    Column {
        id: userCol
        spacing: 15
        anchors.centerIn: parent

        Rectangle {
            width: 120
            height: 120
            anchors.horizontalCenter: parent.horizontalCenter
            color: "transparent"
            radius: 20
            border.width: 2
            border.color: Qt.rgba(1, 1, 1, 0.1)

            D.QtIcon {
                id: avatar
                anchors.fill: parent
                visible: false
            }

            Rectangle {
                id: maskSource
                radius: 20
                anchors.fill: avatar
                visible: false
            }
            D.OpacityMask {
                anchors.fill: avatar
                source: avatar
                maskSource: maskSource
                invert: false
            }
            Rectangle {
                radius: 20
                anchors.fill: avatar
                color: "transparent"
                border.width: 1
                border.color: Qt.rgba(0, 0, 0, 0.2)
            }
        }

        Text {
            text: "User"
            id: username
            font.bold: true
            font.pointSize: 15
            color: "white"
            anchors.horizontalCenter: parent.horizontalCenter
        }

        TextField {
            id: passwordField

            property bool capsIndicatorVisible: false

            width: loginGroup.width
            height: 30
            anchors.horizontalCenter: parent.horizontalCenter
            echoMode: showPasswordBtn.hioceannPWD ? TextInput.Password : TextInput.Normal
            rightPadding: 24
            maximumLength: 510
            placeholderText: qsTr("Please enter password")
            placeholderTextColor: "gray"

            Keys.onPressed: function (event) {
                if (event.key == Qt.Key_CapsLock) {
                    capsIndicatorVisible = !capsIndicatorVisible
                    event.accepted = true
                } else if (event.key == Qt.Key_Return) {
                    userLogin()
                    event.accepted = true
                }
            }

            RowLayout {
                height: parent.height
                anchors.right: parent.right
                anchors.rightMargin: 3

                D.ActionButton {
                    visible: passwordField.capsIndicatorVisible
                    palette.windowText: undefined
                    icon.name: "login_capslock"
                    icon.height: 10
                    icon.width: 10
                    Layout.alignment: Qt.AlignHCenter
                    implicitWidth: 16
                    implicitHeight: 16
                }

                D.ActionButton {
                    id: showPasswordBtn
                    palette.windowText: undefined
                    property bool hioceannPWD: true
                    icon.name: hioceannPWD ? "login_hioceann_password" : "login_display_password"
                    icon.height: 10
                    icon.width: 10
                    Layout.alignment: Qt.AlignHCenter
                    implicitWidth: 16
                    implicitHeight: 16
                    hoverEnabled: true

                    background: Rectangle {
                        anchors.fill: parent
                        radius: 4
                        color: showPasswordBtn.hovered ? Qt.rgba(
                                                             0, 0, 0,
                                                             0.1) : "transparent"
                    }

                    onClicked: hioceannPWD = !hioceannPWD
                }
            }

            background: Rectangle {
                implicitWidth: parent.width
                implicitHeight: parent.height
                color: Qt.rgba(1, 1, 1, 0.6)
                radius: 30
            }
        }
    }

    D.RoundButton {
        id: loginBtn
        icon.name: "login_open"
        icon.width: 16
        icon.height: 16
        height: passwordField.height
        width: height
        anchors.left: userCol.right
        anchors.bottom: userCol.bottom
        anchors.leftMargin: 20
        enabled: passwordField.length != 0
        background: Rectangle {
            anchors.fill: parent
            color: Qt.rgba(255, 255, 255, 0.4)
            radius: parent.height / 2
        }

        onClicked: userLogin()
    }

    RowLayout {
        spacing: 10
        anchors.right: userCol.left
        anchors.bottom: userCol.bottom
        anchors.rightMargin: 10
        height: passwordField.height

        D.RoundButton {
            id: langBtn
            text: 'L'
            visible: false
            Layout.fillHeight: true
            Layout.preferredWidth: passwordField.height
            background: Rectangle {
                anchors.fill: parent
                color: Qt.rgba(255, 255, 255, 0.4)
                radius: parent.height / 2
            }

            onClicked: {
                console.log("need impl langBtn")
            }
        }

        D.RoundButton {
            id: hintBtn
            icon.name: "login_hint"
            icon.width: 16
            icon.height: 16
            visible: hintLabel.hintText.length != 0
            Layout.fillHeight: true
            Layout.preferredWidth: passwordField.height
            background: Rectangle {
                id: hintBtnBackground
                anchors.fill: parent
                color: Qt.rgba(255, 255, 255, 0.4)
                radius: parent.height / 2
            }

            HintLabel {
                id: hintLabel
                x: hintBtn.width - hintLabel.width
                y: hintBtn.height + 11
                hintText: {
                    let user = GreeterModel.userModel.get(GreeterModel.currentUser)
                    return user.passwordHint
                }
            }

            onClicked: hintLabel.open()
        }
    }

    Text {
        id: hintText
        font.pointSize: 10
        color: "gray"
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: userCol.bottom
        anchors.topMargin: 16
    }

    Component.onCompleted: {
        updateUser()
    }

    onActiveFocusChanged: {
        if (activeFocus) passwordField.forceActiveFocus()
    }

    function updateHintMsg(msg) {
        hintText.text = msg
    }

    function userAuthSuccessed() {
        passwordField.text = ""
    }

    function userAuthFailed() {
        passwordField.selectAll()
        if (loginGroup.activeFocus)
            passwordField.forceActiveFocus()
    }
}
