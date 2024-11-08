/*
    SPDX-FileCopyrightText: 2011 Daker Fernandes Pinheiro <dakerfp@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import org.kde.lingmo.components as LingmoComponents
import org.kde.lingmo.extras as LingmoExtras

LingmoComponents.Page {
    height: childrenRect.height
    property int implicitHeight: childrenRect.height

    tools: LingmoComponents.ToolBarLayout {
        spacing: 5
        LingmoComponents.ToolButton {
            visible: pageStack.depth > 1
            iconSource: "go-previous"
            onClicked: pageStack.pop()
        }
        LingmoComponents.Label {
            text: "Text label:"
        }
        LingmoComponents.TextField {
            placeholderText: "Place holder text"
        }
        LingmoComponents.TextField {
            clearButtonShown: true
            text: "Text fields page"
        }
    }

    Flickable {
        id: flickable
        contentWidth: column.width
        contentHeight: column.height
        clip: true
        anchors.fill: parent

        Item {
            width: Math.max(flickable.width, column.width)
            height: column.height
            Column {
                id: column
                spacing: 20
                anchors.horizontalCenter: parent.horizontalCenter

                LingmoComponents.Label {
                    text: "Text Fields"
                    font.pixelSize: 20
                }

                LingmoExtras.Highlight {
                    width: 200
                    height: 100
                    Column {
                        spacing: 10
                        Row {
                            LingmoComponents.Label {
                                text: "Username: "
                                anchors.verticalCenter: tf1.verticalCenter
                            }
                            LingmoComponents.TextField {
                                id: tf1
                                placeholderText: "login"
                                Keys.onTabPressed: tf2.forceActiveFocus();
                            }
                        }

                        Row {
                            LingmoComponents.Label {
                                text: "Password: "
                                anchors.verticalCenter: tf2.verticalCenter
                            }
                            LingmoComponents.TextField {
                                id: tf2
                                width: 120
                                echoMode: TextInput.Password
                                Keys.onTabPressed: loginButton.forceActiveFocus();
                            }
                        }

                        LingmoComponents.Button {
                            id: loginButton
                            text: "Login"
                            anchors {
                                right: parent.right
                                rightMargin: 0
                            }
                            width: 100
                        }
                    }
                }

                LingmoComponents.TextField {
                    width: 120
                    placeholderText: "Disabled Text Field"
                    Keys.onTabPressed: loginButton.forceActiveFocus();
                    enabled: false
                }

                LingmoComponents.Label {
                    text: "Text Area"
                    font.pixelSize: 20
                }

                LingmoComponents.TextArea {
                    width: 200
                    height: 200
                    placeholderText: "Lorem ipsum et dolor"
                    wrapMode: TextEdit.WordWrap
                    contentMaxWidth: 400
                    contentMaxHeight: 400
                }

                LingmoComponents.TextArea {
                    width: 200
                    height: 100
                    enabled: false
                    wrapMode: TextEdit.WordWrap
                    text: "Disabled Text Area"
                }
            }
        }
    }

    LingmoComponents.ScrollBar {
        id: horizontalScrollBar

        flickableItem: flickable
        orientation: Qt.Horizontal
        anchors {
            left: parent.left
            right: verticalScrollBar.left
            bottom: parent.bottom
        }
    }

    LingmoComponents.ScrollBar {
        id: verticalScrollBar

        orientation: Qt.Vertical
        flickableItem: flickable
        anchors {
            top: parent.top
            right: parent.right
            bottom: horizontalScrollBar.top
        }
    }
}
