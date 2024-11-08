/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.
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
 * Authors: youdiansaodongxi <guojiaqi@kylinos.cn>
 *
 */

import QtQuick 2.15
import org.lingmo.calendar 1.0
import QtQuick.Layouts 1.15
import org.lingmo.quick.platform 1.0 as Platform

Item {
    property string stateType: "normal"
    width: 364
    height: 308

    ColumnLayout {
        visible: !monthSelectPage.visible && !yearSelectPage.visible
        anchors.fill: parent
        spacing: 5

        CalendarWeekLabel {
            Layout.alignment: Qt.AlignCenter
        }

        MouseArea {
            width: 364
            height: 288
            hoverEnabled: true

            GridView {
                Layout.alignment: Qt.AlignCenter
                anchors.fill: parent

                cellWidth: 52
                cellHeight: 48
                interactive: false

                model: CalendarModel

                delegate: CalendarItem {
                    day: model.Day
                    lunar: model.LunarDay
                    isCurrentDay: model.IsCurrentDay
                    isCurrentMonth: model.IsCurrentMonth
                }
            }

            onWheel: {
                if (wheel.angleDelta.y > 0) {
                    CalendarModel.displayMonthChange(-1);
                } else {
                    CalendarModel.displayMonthChange(1);
                }
            }
        }
    }

    MouseArea {
        id: yearSelectPage
        property int currentYear

        anchors.fill: parent
        visible: false

        state: "yearSelectPageHid"
        states: [
            State {
                name: "yearSelectPageShowed"
                PropertyChanges {target: yearSelectPage; scale: 1; opacity: 1}
            },
            State {
                name: "yearSelectPageHid"
                PropertyChanges {target: yearSelectPage; scale: 1.2; opacity: 0}
            }
        ]

        transitions: [
            Transition {
                from: "*"; to: "yearSelectPageShowed"
                SequentialAnimation {
                    ScriptAction {
                        script: {
                            yearSelectPage.visible = true;
                            yearSelectPage.currentYear = CalendarModel.getCurrentYear();
                            yearSelectView.currentIndex = CalendarModel.getDisplayYear() - 1974;
                        }
                    }
                    NumberAnimation { properties: "scale,opacity"; easing.type: Easing.InOutCubic; duration: 200}
                }
            },
            Transition {
                from: "*"; to: "yearSelectPageHid"
                SequentialAnimation {
                    NumberAnimation { properties: "scale,opacity"; easing.type: Easing.InOutCubic; duration: 150}
                    ScriptAction {
                        script: {
                            yearSelectPage.visible = false;
                        }
                    }
                }
            }
        ]

        GridView {
            id: yearSelectView
            anchors.centerIn: parent
            width: 300
            height: 240
            clip: true
            cellWidth: 100
            cellHeight: 60
            interactive: false
            model: 120
            cacheBuffer: 120*60

            delegate: MouseArea {
                width: 100
                height: 60
                hoverEnabled: true

                Rectangle {
                    border.color: "blue"
                    border.width: parent.containsMouse ? 1 : 0
                    anchors.fill: parent
                    color: yearSelectPage.currentYear === (index + 1974) ? "lightblue" : "transparent"
                    radius: Platform.Theme.minRadius

                    Text {
                        anchors.centerIn: parent
                        text: (index + 1974)
                    }
                }

                onClicked: {
                    CalendarModel.displayYearChangeTo(index + 1974);
                    stateType = "normal";
                }
            }
        }

        onClicked: {
            stateType = "normal";
        }
        onWheel: {
            if (wheel.angleDelta.y > 0) {
                yearSelectView.currentIndex -= 12;
                if (yearSelectView.currentIndex < 0) yearSelectView.currentIndex = 0;
            } else {
                yearSelectView.currentIndex += 12;
                if (yearSelectView.currentIndex > (119)) yearSelectView.currentIndex = 119;
            }
        }
    }

    MouseArea {
        id: monthSelectPage
        property bool isCurrentYear: true
        property int currentMonth

        anchors.fill: parent
        visible: false

        state: "monthSelectPageHid"
        states: [
            State {
                name: "monthSelectPageShowed"
                PropertyChanges {target: monthSelectPage; scale: 1; opacity: 1}
            },
            State {
                name: "monthSelectPageHid"
                PropertyChanges {target: monthSelectPage; scale: 1.2; opacity: 0}
            }
        ]

        transitions: [
            Transition {
                from: "*"; to: "monthSelectPageShowed"
                SequentialAnimation {
                    ScriptAction {
                        script: {
                            monthSelectPage.visible = true;
                            monthSelectPage.isCurrentYear = CalendarModel.getCurrentYear() === CalendarModel.getDisplayYear();
                            monthSelectPage.currentMonth = CalendarModel.getCurrentMonth();
                        }
                    }
                    NumberAnimation { properties: "scale,opacity"; easing.type: Easing.InOutCubic; duration: 200}
                }
            },
            Transition {
                from: "*"; to: "monthSelectPageHid"
                SequentialAnimation {
                    NumberAnimation { properties: "scale,opacity"; easing.type: Easing.InOutCubic; duration: 150}
                    ScriptAction {
                        script: {
                            monthSelectPage.visible = false;
                        }
                    }
                }
            }
        ]

        GridView {
            anchors.centerIn: parent
            width: 300
            height: 240
            cellWidth: 100
            cellHeight: 60
            interactive: false
            model: ListModel {
                ListElement {index: 1; text: qsTr("Jan")}
                ListElement {index: 2; text: qsTr("Feb")}
                ListElement {index: 3; text: qsTr("Mar")}
                ListElement {index: 4; text: qsTr("Apr")}
                ListElement {index: 5; text: qsTr("May")}
                ListElement {index: 6; text: qsTr("Jun")}
                ListElement {index: 7; text: qsTr("Jul")}
                ListElement {index: 8; text: qsTr("Aug")}
                ListElement {index: 9; text: qsTr("Sep")}
                ListElement {index: 10; text: qsTr("Oct")}
                ListElement {index: 11; text: qsTr("Nov")}
                ListElement {index: 12; text: qsTr("Dec")}
            }

            delegate: MouseArea {
                width: 100
                height: 60
                hoverEnabled: true

                Rectangle {
                    border.color: "blue"
                    border.width: parent.containsMouse ? 1 : 0
                    anchors.fill: parent
                    color: monthSelectPage.isCurrentYear ? monthSelectPage.currentMonth === (model.index) ?
                                                               "lightblue" : "transparent" : "transparent"
                    radius: Platform.Theme.minRadius

                    Text {
                        anchors.centerIn: parent
                        text: model.text
                    }
                }

                onClicked: {
                    CalendarModel.displayMonthChangeTo(model.index);
                    stateType = "normal";
                }
            }
        }

        onClicked: {
            stateType = "normal";
        }
    }

    onStateTypeChanged: {
        if (stateType === "normal") {
            monthSelectPage.state = "monthSelectPageHid";
            yearSelectPage.state = "yearSelectPageHid"
        } else if (stateType == "month") {
            monthSelectPage.state = "monthSelectPageShowed";
            yearSelectPage.state = "yearSelectPageHid"
        } else if (stateType == "year") {
            yearSelectPage.state = "yearSelectPageShowed"
            monthSelectPage.state = "monthSelectPageHid";
        }
    }
}
