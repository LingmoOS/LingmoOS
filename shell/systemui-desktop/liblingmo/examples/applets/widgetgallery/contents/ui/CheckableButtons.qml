/*
    SPDX-FileCopyrightText: 2011 Daker Fernandes Pinheiro <dakerfp@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import org.kde.lingmo.components as LingmoComponents


LingmoComponents.Page {

    tools: LingmoComponents.ToolBarLayout {
        spacing: 5
        LingmoComponents.ToolButton {
            visible: pageStack.depth > 1
            iconSource: "go-previous"
            onClicked: pageStack.pop()
        }
        LingmoComponents.CheckBox {
            text: "Checkbox in the toolbar"
        }
        LingmoComponents.TextField {
            clearButtonShown: true
            text: "hello"
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
                    text: "Check Box"
                }

                LingmoComponents.CheckBox {
                    width: 140
                    height: 30
                    text: "Check Box 1"

                    onCheckedChanged: {
                        if (checked)
                            console.log("CheckBox checked");
                        else
                            console.log("CheckBox unchecked");
                    }
                    onClicked: {
                        console.log("CheckBox clicked");
                    }
                }

                LingmoComponents.CheckBox {
                    height: 30
                    text: "Disabled"
                    enabled: false
                }

                LingmoComponents.CheckBox {
                    height: 30
                    text: ""
                }

                LingmoComponents.CheckBox {
                    height: 30
                    text: "A loooooooooooooong text"
                }

                LingmoComponents.Label {
                    font.pixelSize: 20
                    text: "Radio Button"
                }

                LingmoComponents.RadioButton {
                    width: 140
                    height: 30
                    text: "RadioButton"

                    onCheckedChanged: {
                        if (checked)
                            console.log("RadioButton Checked");
                        else
                            console.log("RadioButton Unchecked");
                    }
                }

                LingmoComponents.Switch { }

                LingmoComponents.Label {
                    font.pixelSize: 20
                    text: "Button Row"
                }

                LingmoComponents.ButtonRow {
                    spacing: 20
                    LingmoComponents.RadioButton { text: "A" }
                    LingmoComponents.RadioButton { text: "B" }
                    LingmoComponents.RadioButton { text: "C" }
                }

                LingmoComponents.Label {
                    font.pixelSize: 20
                    text: "Button Column"
                }

                LingmoComponents.ButtonColumn {
                    spacing: 20
                    LingmoComponents.RadioButton { text: "Alice" }
                    LingmoComponents.RadioButton { text: "Bob" }
                    LingmoComponents.RadioButton { text: "Charles" }
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
