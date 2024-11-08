/*
 * Copyright (C) 2021 LingmoOS Team.
 *
 * Author:     revenmartin <revenmartin@gmail.com>
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

import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import LingmoUI 1.0 as LingmoUI
import Lingmo.Settings 1.0
import "../"

ItemPage {
    headerTitle: qsTr("Date & Time")

    TimeZoneDialog {
        id: timeZoneDialog
    }

    TimeZoneMap {
        id: timeZoneMap
    }

    Time {
        id: time
    }

    Scrollable {
        anchors.fill: parent
        contentHeight: layout.implicitHeight

        ColumnLayout {
            id: layout
            anchors.fill: parent
            spacing: LingmoUI.Units.largeSpacing * 2

            RoundedItem {
                spacing: LingmoUI.Units.largeSpacing * 1.5

                RowLayout {
                    Label {
                        text: qsTr("Auto Sync")
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                    Switch {
                        Layout.fillHeight: true
                        rightPadding: 0
                        rightInset: 0
                        checked: time.useNtp
                        onCheckedChanged: time.useNtp = checked
                    }
                }

                RowLayout {
                    Label {
                        text: qsTr("24-Hour Time")
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                    Switch {
                        Layout.fillHeight: true
                        rightPadding: 0
                        rightInset: 0
                        checked: time.twentyFour
                        onCheckedChanged: time.twentyFour = checked
                    }
                }
            }

            StandardButton {
                Layout.fillWidth: true
                text: ""
                // onClicked: timeZoneDialog.visibility = "Maximized"
                onClicked: timeZoneDialog.show()

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: LingmoUI.Units.largeSpacing * 1.5
                    anchors.rightMargin: LingmoUI.Units.largeSpacing * 1.5

                    Label {
                        text: qsTr("Time Zone")
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                    Label {
                        text: timeZoneMap.currentTimeZone
                    }
                }
            }
        }
    }
}
