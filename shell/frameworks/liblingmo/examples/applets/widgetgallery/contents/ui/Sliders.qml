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
        LingmoComponents.Slider {
            width: 140
            enabled: true
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
                    text: "Slider"
                }


                Column {
                    spacing: 10

                    LingmoComponents.Label { text: "Color Selector"; font.pixelSize: 20 }

                    LingmoComponents.Label { text: "Red" }

                    LingmoComponents.Slider {
                        id: redSlider
                        height: 20
                        width: 255
                        orientation: Qt.Horizontal
                        minimumValue: 0
                        maximumValue: 255
                        stepSize: 10
                        Keys.onTabPressed: greenSlider.forceActiveFocus()
                    }

                    LingmoComponents.Label { text: "Green" }

                    LingmoComponents.Slider {
                        id: greenSlider
                        height: 20
                        width: 255
                        orientation: Qt.Horizontal
                        minimumValue: 0
                        maximumValue: 255
                        tickmarksEnabled: true
                        stepSize: 10
                        Keys.onTabPressed: blueSlider.forceActiveFocus()
                    }

                    LingmoComponents.Label { text: "Blue" }

                    LingmoComponents.Slider {
                        id: blueSlider
                        height: 20
                        width: 255
                        orientation: Qt.Horizontal
                        minimumValue: 0
                        maximumValue: 255
                        stepSize: 10
                        Keys.onTabPressed: redSlider.forceActiveFocus()
                    }

                    Rectangle {
                        anchors.horizontalCenter: parent.horizontalCenter
                        width: parent.width / 2
                        height: width
                        color: Qt.rgba(redSlider.value / 255, greenSlider.value / 255, blueSlider.value / 255, 1)
                    }
                }

                LingmoComponents.Label { text: "Disabled Horizontal Slider" }

                LingmoComponents.Slider {
                    id: horizontalSlider
                    width: 140
                    height: 20
                    enabled: false
                }

                LingmoComponents.Label { text: "Inverted Horizontal Slider" }

                LingmoComponents.Slider {
                    id: invHorizontalSlider
                    width: 140
                    height: 20
                    inverted: true
                    enabled: true
                }

                LingmoComponents.Label { text: "Vertical Slider" }

                Row {
                    spacing: 30
                    LingmoComponents.Slider {
                        id: verticalSlider
                        width: 20
                        height: 140
                        orientation: Qt.Vertical
                        minimumValue: 10
                        maximumValue: 1000
                        stepSize: 50
                        inverted: true
                    }
                    LingmoComponents.Label { text: verticalSlider.value }
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
