// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.

// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.15
import org.deepin.dtk 1.0
import "../Utils"

Item {
    Item {
        id: header
        height: Utils.welcomeHeaderHeight
        x: Utils.margin
        RowLayout {
            spacing: Utils.spacing * 5
            ColumnLayout {
                Image {
                    source: "/images/logo.svg"
                }
                Text {
                    text: qsTr('Now,\nYou can control.')
                    font: DTK.fontManager.t2
                }
            }
            // TODO: use lottie qml
            AnimatedImage {
                source: "/images/welcome.webp"
                Layout.preferredWidth: header.height
                Layout.preferredHeight: header.height
            }
        }
        Behavior on height {
            PropertyAnimation {
                duration: Utils.welcomeHeaderAnimateDuration
                easing.type: Easing.InOutQuad
            }
        }
    }

    Text {
        id: text
        text: qsTr("Welcome to the deepin advanced tool -- Deepin Tweak! \nYou can define deepin in an advanced way by selecting the desired items in the sidebar. From the desktop environment to the kernel, Deepin Tweak gives you a high degree of customizability. Now, let's explore! \nIf you have more suggestions for this software, please feel free to submit a Pull Request or Issue; if this software has helped you, you can consider donating to us, or why not also give us a Star for our project. your support is our motivation to continue development, we appreciate it.")
        clip: true
        wrapMode: Text.WordWrap
        x: Utils.margin
        width: parent.width - x * 2
        anchors.top: header.bottom
        anchors.bottom: bottomWrapper.top
        anchors.topMargin: Utils.margin
        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            onEntered: {
                header.height = Utils.welcomeHeaderAnimatedHeight
            }
            onExited: {
                header.height = Utils.welcomeHeaderHeight
            }
        }
    }

    Rectangle {
        color: Utils.transparentBackground
        anchors.bottom: parent.bottom
        x: Utils.margin
        width: parent.width - x * 2
        height: bottom.height
        id: bottomWrapper
        RowLayout {
            spacing: Utils.spacing
            id: bottom
            width: parent.width
            Image {
                source: "/images/github_fill.svg"
                MouseArea {
                    anchors.fill: parent
                    onClicked: Qt.openUrlExternally("https://github.com/linuxdeepin/")
                }
            }
            Image {
                source: "/images/deepin-logo.svg"
                MouseArea {
                    anchors.fill: parent
                    onClicked: Qt.openUrlExternally("https://www.deepin.org/")
                }
            }
            Item {
                Layout.fillWidth: true
            }
            Text {
                text: qsTr("Made by deepin Opensource Community with love.")
                font: DTK.fontManager.t9
            }
        }
    }


    Rectangle {
        id: headerBackground
        color: palette.alternateBase
        width: parent.width
        height: parent.height
        z: -1
        anchors.top: header.bottom
        radius: Utils.radius
    }
}

