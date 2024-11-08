/*
    SPDX-FileCopyrightText: 2011 Daker Fernandes Pinheiro <dakerfp@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import org.kde.lingmo.components as LingmoComponents

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
        LingmoComponents.ProgressBar {
            value: 0.3
        }
        LingmoComponents.TextField {
            clearButtonShown: true
            text: "Busy widgets"
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
                    text: "Busy Indicator"
                }

                LingmoComponents.BusyIndicator { }

                LingmoComponents.BusyIndicator { running: true }

                LingmoComponents.Label {
                    font.pixelSize: 20
                    text: "Progress Bar"
                }

                LingmoComponents.Label { text: "Horizontal" }

                LingmoComponents.ProgressBar {
                    value: 0.3
                }

                LingmoComponents.ProgressBar {
                    indeterminate: true
                }

                LingmoComponents.ProgressBar {
                    minimumValue: 0
                    maximumValue: 100
                    value: 30
                }

                LingmoComponents.Label { text: "Vertical" }
                Row {
                    spacing: 20
                    LingmoComponents.ProgressBar {
                        value: 0.3
                        orientation: Qt.Vertical
                        width: 20
                        height: 100
                    }
                    LingmoComponents.ProgressBar {
                        value: 0.4
                        orientation: Qt.Vertical
                        width: 20
                        height: 120
                    }
                    LingmoComponents.ProgressBar {
                        orientation: Qt.Vertical
                        width: 20
                        height: 100
                        indeterminate: true
                    }
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
