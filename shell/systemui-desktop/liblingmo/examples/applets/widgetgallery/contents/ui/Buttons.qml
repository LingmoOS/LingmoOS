/*
    SPDX-FileCopyrightText: 2011 Daker Fernandes Pinheiro <dakerfp@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Controls as QQC2
import org.kde.lingmo.components as LingmoComponents

LingmoComponents.Page {
    height: childrenRect.height
    tools: LingmoComponents.ToolBarLayout {
        spacing: 5
        LingmoComponents.ToolButton {
            visible: pageStack.depth > 1
            icon.name: "go-previous"
            onClicked: pageStack.pop()
        }
        LingmoComponents.Button {
            text: "Button"
        }
        LingmoComponents.TextField {
            clearButtonShown: true
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
                    font.pixelSize: 20
                    text: "Buttons"
                }

                LingmoComponents.Button {
                    id: bt1

                    text: "Button"

                    onClicked: {
                        console.log("Clicked");
                        pageStack.push(Qt.createComponent("Scrollers.qml"))
                    }

                    Keys.onTabPressed: bt2.forceActiveFocus();
                }

                LingmoComponents.Button {
                    id: bt2

                    text: "Checkable Button"
                    checkable: true

                    onCheckedChanged: {
                        if (checked)
                            console.log("Button Checked");
                        else
                            console.log("Button Unchecked");
                    }

                    Keys.onTabPressed: bt3.forceActiveFocus();
                }

                LingmoComponents.Button {
                    id: bt3

                    text: "Different Font"
                    font {
                        pointSize: 20
                        family: "Helvetica"
                    }

                    Keys.onTabPressed: bt4.forceActiveFocus();
                }

                LingmoComponents.Button {
                    id: bt4

                    text: "Icon Button"
                    icon.name: "konqueror"

                    Keys.onTabPressed: bt5.forceActiveFocus();
                    menu: QQC2.Menu {
                        QQC2.MenuItem { text: "This Button" }
                        QQC2.MenuItem { text: "Happens To Have" }
                        QQC2.MenuItem { text: "A Menu Assigned" }
                    }
                }

                LingmoComponents.Button {
                    id: bt5

                    icon.name: "lingmo"

                    Keys.onTabPressed: bt1.forceActiveFocus();
                }

                LingmoComponents.Button {

                    text: "Disabled Button"
                    enabled: false
                }

                LingmoComponents.ToolButton {
                    text: "ToolButton"
                }

                LingmoComponents.ToolButton {
                    text: "ToolButton not flat"
                    flat: false
                }

                LingmoComponents.ToolButton {

                    text: "Icon ToolButton"
                    icon.name: "konqueror"
                }

                LingmoComponents.ToolButton {
                    icon.name: "lingmo"
                }
                LingmoComponents.ToolButton {
                    icon.name: "lingmo"
                    flat: false
                }

                LingmoComponents.ToolButton {
                    text: "Disabled ToolButton"
                    enabled: false
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
