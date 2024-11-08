/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: hxf <hewenfei@kylinos.cn>
 */

import QtQuick 2.12
import QtQuick.Layouts 1.12
import org.lingmo.menu.aiAssistant 1.0
import QtQuick.Controls 2.12

Item {
    id: root
    property var session: null

    Component {
        id: openContextMenu
        Item {
            Image {
                anchors.centerIn: parent
                width: 16
                height: 16
                source: "qrc:///ernie-bot/content/icon/menu.svg"
            }
            MouseArea {
                anchors.fill: parent
                acceptedButtons: Qt.LeftButton
                onClicked: {
                    sessionSubMenu.contentItem.model = extensionData.sessionManager.sessionIds();
                    sessionMenu.popup();
                }
            }
        }
    }
    function updateSession() {
        var sessionId = extensionData.messageModel.currentSessionId;
        if (session !== null) {
            session.sessionIsReadyChanged.disconnect(pauseSession);
            session.responseIsReadyChanged.disconnect(pauseSession);
        }

        session = extensionData.sessionManager.getSession(sessionId);
        if (session !== null) {
            session.sessionIsReadyChanged.connect(pauseSession);
            session.responseIsReadyChanged.connect(pauseSession);
            pauseSession();
        }
    }

    function pauseSession() {
        sessionUnReadyItem.visible =!session.sessionIsReady;
        messageInput.disableInput = !session.responseIsReady || !session.sessionIsReady;
    }

    Component.onCompleted: {
        messageView.model = extensionData.messageModel
        if (extensionData.messageModel.currentSessionId === -1) {
            extensionData.sessionManager.newSession();
        }

        updateSession();
        extensionData.messageModel.currentSessionIdChanged.connect(updateSession);
        mainPage.extensionMenu = openContextMenu;
    }

    Component.onDestruction: {
        extensionData.messageModel.currentSessionIdChanged.disconnect(updateSession);
        mainPage.extensionMenu = null;
    }

    ContextMenu {
        id: sessionMenu
        width: 120

        ContextMenu {
            id: sessionSubMenu
            title: qsTr("Select Session")
            width: 120

            contentItem: ListView {
                width: sessionSubMenu.width
                spacing: 0
                onContentHeightChanged: {
                    height = childrenRect.height;
                    sessionSubMenu.height = height + sessionSubMenu.padding*2;
                }

                delegate: Rectangle {
                    width: ListView.view.width
                    height: 30
                    radius: 4
                    color: mouseArea.containsMouse ? "#3790FA" : "#ffffff"

                    Rectangle {
                        width: 8; height: 8
                        anchors.left: parent.left
                        anchors.leftMargin: 4
                        anchors.verticalCenter: parent.verticalCenter
                        radius: 4
                        visible: modelData === extensionData.messageModel.currentSessionId
                        color: mouseArea.containsMouse ? "#ffffff" : "#3790FA"
                    }

                    Text {
                        anchors.fill: parent
                        anchors.leftMargin: 16
                        anchors.rightMargin: 6
                        text: extensionData.sessionManager.getSession(modelData).sessionName;
                        color: mouseArea.containsMouse ? "#ffffff" : "#000000"
                        verticalAlignment: Text.AlignVCenter
                        elide: Text.ElideMiddle
                    }

                    MouseArea {
                        id: mouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        onClicked: {
                            extensionData.messageModel.setSession(extensionData.sessionManager.getSession(modelData));
                            sessionMenu.close();
                        }
                    }
                }
            }
        }

        MenuSeparator {
            contentItem: Rectangle {
                implicitWidth: 120
                implicitHeight: 1
                color: "#dedede"
            }
        }

        Action {
            text: qsTr("New Session")
            onTriggered: {
                extensionData.sessionManager.newSession();
            }
        }

        Action {
            text: qsTr("Clear Session")
            onTriggered: {
                root.session.clearSession()
            }
        }

        Action {
            text: qsTr("Exit ID")
            onTriggered: {
                extensionData.sessionManager.exitSessionApi();
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.leftMargin: 8
        anchors.rightMargin: 8
        anchors.bottomMargin: 8

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            Loader {
                id: sessionUnReadyItem
                anchors.centerIn: parent
                width: 50; height: 50
                visible: false
                z: 100

                active: visible
                focus: false
                sourceComponent: Component {
                    Item {
                        Component.onCompleted: {
                            animation.start();
                        }

                        RotationAnimation {
                            id: animation
                            target: pathView
                            to: 360
                            duration: 1500
                            direction: RotationAnimation.Clockwise

                            onStopped: {
                                if (pathView.rotation === 360) {
                                    pathView.rotation = 0;
                                }

                                start();
                            }
                        }

                        PathView {
                            id: pathView
                            anchors.fill: parent

                            path: Path {
                                startX: 25; startY: 0
                                PathArc {
                                    x: 25; y: 50
                                    radiusX: 25
                                    radiusY: 25
                                    useLargeArc: true
                                }
                                PathArc {
                                    x: 25; y: 0
                                    radiusX: 25
                                    radiusY: 25
                                    useLargeArc: true
                                }
                            }
                            interactive: false
                            model: 8
                            delegate: Rectangle {
                                width: 10; height: 10
                                radius: 5
                                color: "#BF3790FA"
                            }
                        }
                    }
                }
            }

            ListView {
                id: messageView
                anchors.fill: parent
                z: 50

                focus: true
                clip: true
                // model: extensionData.messageModel
                spacing: 10
                cacheBuffer: count * 100
                boundsBehavior: Flickable.StopAtBounds
                highlightMoveDuration: 500
                highlightMoveVelocity: -1
                onCountChanged: {
                    currentIndex = count - 1;
                }

                delegate: Loader {
                    property string header: model.header
                    property string content: model.content
                    property var actions: model.actions
                    property var view: ListView.view

                    width: ListView.view.width
                    height: childrenRect.height
                    focus: true

                    sourceComponent: {
                        let type = model.type;

                        if (type === Message.SentMessage) {
                            return sentMessage;
                        }

                        if (type === Message.ReceivedMessage) {
                            return receivedMessage;
                        }

                        if (type === Message.TimeMessage) {
                            return timeMessage;
                        }

                        if (type === Message.TipMessage) {
                            return tipMessage;
                        }

                        if (type === Message.LoadTipMessage) {
                            return loading;
                        }

                        if (type === Message.ErrorMessage) {
                            return errorMessage;
                        }
                    }
                }

                Component {
                    id: loading
                    Item {
                        height: childrenRect.height
                        LoadTipMessage {
                            width: 100
                            height: 45
                            anchors.left: parent.left
                        }
                    }
                }

                Component {
                    id: sentMessage
                    MessageBar {
                        color: "#3790FA"
                        radius: 6

                        messageColor: "#ffffff"
                        selectionColor: Qt.rgba(0, 0, 0, 1)
                    }
                }

                Component {
                    id: receivedMessage
                    MessageBar {
                        alignRight: false
                        color: Qt.rgba(255, 255, 255, 0.45)
                        radius: 6

                        border.width: 1
                        border.color: Qt.rgba(0, 0, 0, 0.1)

                        messageColor: "#000000"
                        selectionColor: "#3790FA"
                    }
                }

                Component {
                    id: errorMessage
                    Rectangle {
                        height: childrenRect.height
                        radius: 6

                        color: Qt.rgba(255, 255, 255, 0.45)
                        border.width: 1
                        border.color: Qt.rgba(0, 0, 0, 0.1)

                        Column {
                            width: parent.width
                            height: childrenRect.height
                            Item {
                                width: parent.width
                                height: errorMessageBar.height

                                Image {
                                    width: 16
                                    height: 16
                                    anchors.left: parent.left
                                    anchors.leftMargin: 12
                                    anchors.verticalCenter: parent.verticalCenter

                                    enabled: !root.disableInput;
                                    source: "qrc:///ernie-bot/content/icon/warn.svg"
                                }

                                MessageBar {
                                    id: errorMessageBar
                                    anchors.left: parent.left
                                    anchors.right: parent.right
                                    anchors.leftMargin: 28
                                    alignRight: false

                                    color: "transparent"
                                    radius: 6

                                    messageColor: "#3790FA"
                                    selectionColor: "#808080"
                                }
                            }

                            Rectangle {
                                width: parent.width
                                height: 1
                                color: Qt.rgba(38, 38, 38, 0.2)
                            }

                            Item {
                                width: parent.width
                                height: 40
                                visible: errorActions.count > 0

                                ListView {
                                    id: errorActions
                                    anchors.horizontalCenter: parent.horizontalCenter
                                    width: childrenRect.width
                                    height: parent.height

                                    orientation: ListView.Horizontal
                                    model: actions
                                    delegate: Item {
                                        width: 80
                                        height: ListView.view.height

                                        Text {
                                            anchors.fill: parent
                                            text: modelData.name
                                            verticalAlignment: Text.AlignVCenter
                                            horizontalAlignment: Text.AlignHCenter
                                            elide: Text.ElideRight
                                            color: "#3790FA"
                                        }

                                        MouseArea {
                                            anchors.fill: parent
                                            onClicked: {
                                                modelData.exec();
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                Component {
                    id: timeMessage
                    TimeMessage {}
                }

                Component {
                    id: tipMessage
                    TipMessage {}
                }
            }

            MouseArea {
                anchors.fill: parent
                acceptedButtons: Qt.LeftButton | Qt.RightButton
                onClicked: {
                    if (mouse.button === Qt.RightButton) {
                        sessionSubMenu.contentItem.model = extensionData.sessionManager.sessionIds();
                        sessionMenu.popup();
                    }
                }
            }
        }

        MessageInput {
            id: messageInput
            Layout.fillWidth: true
            Layout.preferredHeight: 40

            radius: 6

            onMessageSend: function(message) {
                messageView.model.talk(message);
            }
        }
    }
}
