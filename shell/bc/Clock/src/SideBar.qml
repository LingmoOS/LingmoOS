/*
 * Copyright (C) 2024 Lingmo OS Team.
 *
 * Author:     Lingmo OS Team <team@lingmo.org>
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

import QtQuick 2.4
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.0
import LingmoUI 1.0 as LingmoUI

Item {
    implicitWidth: 230

    property int itemRadiusV: 8

    property alias view: listView
    property alias model: listModel
    property alias currentIndex: listView.currentIndex

    Rectangle {
        anchors.fill: parent
        color: LingmoUI.Theme.darkMode ? Qt.lighter(LingmoUI.Theme.backgroundColor, 1.5)
                                     : Qt.darker(LingmoUI.Theme.backgroundColor, 1.05)
        opacity: rootWindow.compositing ? 0.3 : 0.4

        Behavior on color {
            ColorAnimation {
                duration: 250
                easing.type: Easing.Linear
            }
        }
    }

    ListModel {
        id: listModel

        ListElement {
            title: qsTr("时钟")
            name: "clock"
            iconSource: "clock.svg"
            page: "qrc:/AnalogClock.qml"
        }

        ListElement {
            title: qsTr("世界时钟")
            name: "worldClock"
            iconSource: "worldClock.svg"
            page: "qrc:/WorldClockView.qml"
        }
        ListElement {
            title: qsTr("闹钟")
            iconSource: "alarm.svg"
            page: "qrc:/AlarmView.qml"
        }
        ListElement {
            title: qsTr("计时器")
            name: "timer"
            iconSource: "timer.svg"
            page: "qrc:/TimerView.qml"
        }
        ListElement {
            title: qsTr("秒表")
            name: "stopwatch"
            iconSource: "stopwatch.svg"
            page: "qrc:/StopwatchView.qml"
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 0
        spacing: 0

        Label {
            text: rootWindow.title
            color: rootWindow.active ? LingmoUI.Theme.textColor : LingmoUI.Theme.disabledTextColor
            Layout.preferredHeight: rootWindow.header.height
            leftPadding: LingmoUI.Units.largeSpacing + LingmoUI.Units.smallSpacing
            rightPadding: LingmoUI.Units.largeSpacing + LingmoUI.Units.smallSpacing
            topPadding: LingmoUI.Units.smallSpacing
            bottomPadding: 0
            font.pointSize: 13
        }

        ListView {
            id: listView
            Layout.fillHeight: true
            Layout.fillWidth: true
            clip: true
            model: listModel

            spacing: LingmoUI.Units.smallSpacing
            leftMargin: LingmoUI.Units.largeSpacing
            rightMargin: LingmoUI.Units.largeSpacing
            topMargin: 0
            bottomMargin: LingmoUI.Units.largeSpacing

            ScrollBar.vertical: ScrollBar {}

            highlightFollowsCurrentItem: true
            highlightMoveDuration: 0
            highlightResizeDuration : 0
            highlight: Rectangle {
                radius: LingmoUI.Theme.mediumRadius
                color: LingmoUI.Theme.highlightColor
                smooth: true
            }

            LingmoUI.WheelHandler {
                target: listView
            }

            delegate: Item {
                id: item
                width: ListView.view.width - ListView.view.leftMargin - ListView.view.rightMargin
                height: LingmoUI.Units.fontMetrics.height + LingmoUI.Units.largeSpacing * 1.5

                property bool isCurrent: listView.currentIndex === index

                Rectangle {
                    anchors.fill: parent

                    MouseArea {
                        id: mouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        acceptedButtons: Qt.LeftButton
                        onClicked: listView.currentIndex = index
                    }

                    radius: LingmoUI.Theme.mediumRadius
                    color: mouseArea.containsMouse && !isCurrent ? Qt.rgba(LingmoUI.Theme.textColor.r,
                                                                           LingmoUI.Theme.textColor.g,
                                                                           LingmoUI.Theme.textColor.b,
                                                                   0.1) : "transparent"

                    smooth: true
                }

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: LingmoUI.Units.largeSpacing
                    spacing: LingmoUI.Units.smallSpacing * 1.5

                    Rectangle {
                        id: iconRect
                        width: 24
                        height: 24
                        Layout.alignment: Qt.AlignVCenter
                        radius: 20
                        color: LingmoUI.Theme.highlightColor

                        Image {
                            id: icon
                            anchors.centerIn: parent
                            width: 16
                            height: width
                            source: "qrc:/images/" + model.iconSource
                            sourceSize: Qt.size(width, height)
                            Layout.alignment: Qt.AlignVCenter
                            antialiasing: false
                            smooth: false
                        }
                    }

                    Label {
                        id: itemTitle
                        text: model.title
                        color: isCurrent ? LingmoUI.Theme.highlightedTextColor : LingmoUI.Theme.textColor
                    }

                    Item {
                        Layout.fillWidth: true
                    }
                }
            }

        }
    }

    function removeItem(name) {
        for (var i = 0; i < listModel.count; ++i) {
            if (name === listModel.get(i).name) {
                listModel.remove(i)
                break
            }
        }
    }
}
