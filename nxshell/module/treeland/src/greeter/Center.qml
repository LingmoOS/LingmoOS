// Copyright (C) 2023 justforlxz <justforlxz@gmail.com>.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Dialogs
import org.lingmo.dtk 1.0 as D
import TreeLand
import TreeLand.Utils

FocusScope {
    id: root

    property UserInput loginGroup: userInput

    Loader {
        id: leftAnimation
    }

    Component {
        id: leftAnimationComponent

        LoginAnimation {
            onStopped: {
            }
        }
    }

    Loader {
        id: rightAnimation
    }

    Component {
        id: rightAnimationComponent

        LoginAnimation {
            onStopped: {
                GreeterModel.emitAnimationPlayFinished()
            }
        }
    }

    Connections {
        target: GreeterModel
        function onAnimationPlayed() {
            leftAnimation.parent = quickActions.parent
            leftAnimation.anchors.fill = quickActions
            leftAnimation.sourceComponent = leftAnimationComponent
            leftAnimation.item.start(quickActions,{x: quickActions.x}, {x: -quickActions.width})

            rightAnimation.parent = right.parent
            rightAnimation.anchors.fill = right
            rightAnimation.sourceComponent = rightAnimationComponent
            rightAnimation.item.start(right, {x: 0}, {x: right.width})
        }
    }

    Item {
        id: quickActions
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: root.width * 0.4

        TimeDateWidget {
            currentLocale :{
                let user = GreeterModel.userModel.get(GreeterModel.currentUser)
                return user.locale
            }
            anchors.right: parent.right
            anchors.rightMargin: parent.width / 10
            anchors.verticalCenter: parent.verticalCenter
            width: 400
            height: 156
            background: Rectangle {
                radius: 8
                color: "white"
                opacity: 0.1
            }
        }
    }

    Rectangle {
        width: logo.implicitWidth
        height: logo.implicitHeight
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        color: "transparent"

        RowLayout {
            id: logo
            anchors.fill: parent
            spacing: 5

            Image {
                id: logoPic
                Layout.alignment: Qt.AlignBottom | Qt.AlignLeft
                source: GreeterModel.logoProvider.logo
                Layout.maximumHeight: 132
                Layout.maximumWidth: 128
                fillMode: Image.PreserveAspectFit
            }

            Text {
                Layout.alignment: Qt.AlignLeft
                text: GreeterModel.logoProvider.version
                font.weight: Font.Normal
                font.pixelSize: logoPic.height / 2
                color: Qt.rgba(1, 1, 1, 153 / 255)
            }
        }
    }

    FocusScope {
        id: right
        anchors.left: quickActions.right
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom

        focus: true

        UserInput {
            id: userInput
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.leftMargin: parent.width / 6

            focus: true
        }

        //TODO: abstract ot a Button type
        RowLayout {
            id: bottomGroup
            property var buttonSize: 30
            anchors.bottom: parent.bottom
            anchors.right: parent.right
            spacing: 15

            Rectangle {
                implicitWidth: bottomGroup.buttonSize + 6
                implicitHeight: bottomGroup.buttonSize + 6
                color: "transparent"
                visible: false
                Layout.alignment: Qt.AlignHCenter
                RoundButton {
                    id: wifiBtn
                    text: '1'
                    width: bottomGroup.buttonSize
                    height: bottomGroup.buttonSize
                    anchors.centerIn: parent
                    onClicked: {
                        console.log("need impl btn 1")
                    }

                    background: RoundBlur {}
                }
            }

            //NOTE: Remove this on later
            MessageDialog {
                id: backToNormalDialog
                buttons: MessageDialog.Ok | MessageDialog.Cancel
                text: qsTr("Return to default mode")
                informativeText: qsTr("Do you want to back to default?")
                detailedText: qsTr("This action will reboot machine, please confirm.")
                onButtonClicked: function (button, role) {
                    switch (button) {
                    case MessageDialog.Ok:
                        Helper.backToNormal()
                        Helper.reboot()
                        break
                    }
                }
            }

            Rectangle {
                implicitWidth: bottomGroup.buttonSize + 6
                implicitHeight: bottomGroup.buttonSize + 6
                color: "transparent"
                Layout.alignment: Qt.AlignHCenter
                D.RoundButton {
                    id: boardBtn
                    icon.name: "backToLightdm"
                    icon.width: 16
                    icon.height: 16
                    width: bottomGroup.buttonSize
                    height: bottomGroup.buttonSize
                    anchors.centerIn: parent

                    D.ToolTip.visible: hovered
                    D.ToolTip.text: qsTr("Back To Default Mode")

                    onClicked: backToNormalDialog.open()
                    background: RoundBlur {}
                }
            }

            Rectangle {
                implicitWidth: bottomGroup.buttonSize + 6
                implicitHeight: bottomGroup.buttonSize + 6
                color: "transparent"
                Layout.alignment: Qt.AlignHCenter
                visible: userList.count > 1

                D.RoundButton {
                    id: usersBtn

                    property bool expand: false
                    icon.name: "login_user"
                    icon.width: 16
                    icon.height: 16
                    width: expand ? bottomGroup.buttonSize + 6 : bottomGroup.buttonSize
                    height: expand ? bottomGroup.buttonSize + 6 : bottomGroup.buttonSize
                    anchors.centerIn: parent
                    hoverEnabled: parent.visible

                    D.ToolTip.visible: hovered
                    D.ToolTip.text: qsTr("Other Users")

                    UserList {
                        id: userList
                        x: (usersBtn.width - userList.width) / 2 - 10
                        y: -userList.height - 10
                        onClosed: usersBtn.expand = false
                    }

                    onClicked: {
                        usersBtn.expand = true
                        userList.open()
                    }

                    background: RoundBlur {}
                }
            }

            Rectangle {
                implicitWidth: bottomGroup.buttonSize + 6
                implicitHeight: bottomGroup.buttonSize + 6
                color: "transparent"
                Layout.alignment: Qt.AlignHCenter
                D.RoundButton {
                    id: powerBtn

                    property bool expand: false
                    icon.name: "login_power"
                    icon.width: 16
                    icon.height: 16
                    width: expand ? bottomGroup.buttonSize + 6 : bottomGroup.buttonSize
                    height: expand ? bottomGroup.buttonSize + 6 : bottomGroup.buttonSize
                    anchors.centerIn: parent
                    D.ToolTip.visible: hovered
                    D.ToolTip.text: qsTr("Power")
                    PowerList {
                        id: powerList
                        y: -powerList.height - 10
                        x: (powerBtn.width - powerList.width) / 2 - 10
                        onClosed: powerBtn.expand = false
                    }
                    onClicked: {
                        powerBtn.expand = true
                        powerList.open()
                    }

                    background: RoundBlur {}
                }
            }
        }
    }
}
