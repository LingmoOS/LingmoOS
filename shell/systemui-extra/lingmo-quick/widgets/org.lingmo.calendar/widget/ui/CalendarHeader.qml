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
import QtQuick.Layouts 1.15
import org.lingmo.calendar 1.0
import org.lingmo.quick.items 1.0 as LingmoUIItems
import org.lingmo.quick.platform 1.0

Item {
    RowLayout {
        anchors.fill: parent
        Row {
            Layout.alignment: Qt.AlignLeft
            MouseArea {
                width: 24
                height: 24

                LingmoUIItems.Icon {
                    anchors.centerIn: parent
                    width: 16
                    height: 16
                    source: "lingmo-start-symbolic"
                    mode: LingmoUIItems.Icon.AutoHighlight
                }

                onClicked: {
                    CalendarModel.displayMonthChange(-1);
                }
            }
            Item {
                width: 62
                height: 24
                Text {
                    id: dateLabel
                    anchors.centerIn: parent
                    text: CalendarModel.getDisplayYear() + "." + CalendarModel.getDisplayMonth()

                    Component.onCompleted: {
                        CalendarModel.displayDateChanged.connect(updateText);
                    }
                    Component.onDestruction: {
                        CalendarModel.displayDateChanged.disconnect(updateText);
                    }

                    function updateText() {
                        dateLabel.text = CalendarModel.getDisplayYear() + "." + CalendarModel.getDisplayMonth();
                    }
                }
            }
            MouseArea {
                width: 24
                height: 24

                LingmoUIItems.Icon {
                    anchors.centerIn: parent
                    width: 16
                    height: 16
                    source: "lingmo-end-symbolic"
                    mode: LingmoUIItems.Icon.AutoHighlight
                }

                onClicked: {
                    CalendarModel.displayMonthChange(1);
                }
            }
        }

        Row {
            spacing: 8
            Layout.alignment: Qt.AlignRight
            MouseArea {
                width: 44
                height: 28

                LingmoUIItems.StyleBackground {
                    anchors.fill: parent
                    useStyleTransparency: false
                    paletteRole: Theme.BrightText
                    alpha: 0.05
                    Text {
                        anchors.centerIn: parent
                        text: qsTr("year")
                    }
                }

                onClicked: {
                    root.calendarStateChange("year");
                }
            }

            MouseArea {
                width: 44
                height: 28

                LingmoUIItems.StyleBackground {
                    anchors.fill: parent
                    useStyleTransparency: false
                    paletteRole: Theme.BrightText
                    alpha: 0.05
                    Text {
                        anchors.centerIn: parent
                        text: qsTr("month")
                    }
                }

                onClicked: {
                    root.calendarStateChange("month");
                }
            }

            MouseArea {
                width: 44
                height: 28

                LingmoUIItems.StyleBackground {
                    anchors.fill: parent
                    useStyleTransparency: false
                    paletteRole: Theme.BrightText
                    alpha: 0.05
                    Text {
                        anchors.centerIn: parent
                        text: qsTr("today")
                    }
                }

                onClicked: {
                    CalendarModel.displayMonthChange(0);
                    root.calendarStateChange("normal");
                }
            }

            LingmoUIItems.StyleBackground {
                useStyleTransparency: false
                paletteRole: Theme.BrightText
                alpha: 0.05
                width: 28
                height: 28

                LingmoUIItems.Icon {
                    anchors.centerIn: parent
                    width: 16
                    height: 16
                    source: "lingmo-down-symbolic"
                    mode: LingmoUIItems.Icon.AutoHighlight
                }
            }
        }
    }
}
