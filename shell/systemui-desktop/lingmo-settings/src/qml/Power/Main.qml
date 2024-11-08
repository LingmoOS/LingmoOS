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

import QtQuick 2.4
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.0

import Lingmo.Settings 1.0
import LingmoUI 1.0 as LingmoUI
import "../"

ItemPage {
    id: control
    headerTitle: qsTr("Power")

    PowerManager {
        id: power
    }

    Battery {
        id: battery
    }

    function timeoutToIndex(timeout) {
        switch (timeout) {
        case 2 * 60:
            return 0
        case 5 * 60:
            return 1
        case 10 * 60:
            return 2
        case 15 * 60:
            return 3
        case 30 * 60:
            return 4
        case -1:
            return 5
        }
    }

    Scrollable {
        anchors.fill: parent
        contentHeight: layout.implicitHeight

        ColumnLayout {
            id: layout
            anchors.fill: parent
            spacing: LingmoUI.Units.largeSpacing

            RoundedItem {
                Label {
                    text: qsTr("Mode")
                    color: LingmoUI.Theme.disabledTextColor
                }

                RowLayout {
                    spacing: LingmoUI.Units.largeSpacing * 2

                    IconCheckBox {
                        source: "qrc:/images/powersave.svg"
                        text: qsTr("Power Save")
                        checked: power.mode === 0
                        onClicked: power.mode = 0
                    }

//                    IconCheckBox {
//                        source: "qrc:/images/balance.svg"
//                        text: qsTr("Balance")
//                        checked: false
//                    }

                    IconCheckBox {
                        source: "qrc:/images/performance.svg"
                        text: qsTr("Performance")
                        checked: power.mode === 1
                        onClicked: power.mode = 1
                    }
                }
            }

            Label {
                color: LingmoUI.Theme.disabledTextColor
                leftPadding: LingmoUI.Units.largeSpacing * 2
                rightPadding: LingmoUI.Units.largeSpacing
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                text: qsTr("Performance mode: CPU and GPU frequencies will be increased, while power consumption and heat generation will be increased.")
            }

            RoundedItem {
                Layout.topMargin: LingmoUI.Units.largeSpacing

                GridLayout {
                    columns: 2
                    rowSpacing: LingmoUI.Units.largeSpacing * 2
                    Layout.bottomMargin: LingmoUI.Units.largeSpacing

                    Label {
                        text: qsTr("Turn off screen")
                        Layout.fillWidth: true
                    }

                    ComboBox {
                        Layout.preferredWidth: 160

                        model: ListModel {
                            ListElement { text: qsTr("2 Minutes") }
                            ListElement { text: qsTr("5 Minutes") }
                            ListElement { text: qsTr("10 Minutes") }
                            ListElement { text: qsTr("15 Minutes") }
                            ListElement { text: qsTr("30 Minutes") }
                            ListElement { text: qsTr("Never") }
                        }

                        Component.onCompleted: {
                            currentIndex = timeoutToIndex(power.idleTime)
                        }

                        onActivated: {
                            switch (currentIndex) {
                            case 0:
                                power.idleTime = 2 * 60
                                break
                            case 1:
                                power.idleTime = 5 * 60
                                break
                            case 2:
                                power.idleTime = 10 * 60
                                break
                            case 3:
                                power.idleTime = 15 * 60
                                break
                            case 4:
                                power.idleTime = 30 * 60
                                break
                            case 5:
                                power.idleTime = -1
                                break
                            }
                        }
                    }

                    Label {
                        text: qsTr("Hibernate after screen is turned off")
                        Layout.fillWidth: true
                    }

                    Switch {
                        Layout.fillHeight: true
                        Layout.alignment: Qt.AlignRight
                        checked: power.sleepWhenClosedScreen
                        onClicked: power.sleepWhenClosedScreen = checked
                    }

                    Label {
                        text: qsTr("Lock screen after screen is turned off")
                        Layout.fillWidth: true
                    }

                    Switch {
                        Layout.fillHeight: true
                        Layout.alignment: Qt.AlignRight
                        checked: power.lockWhenClosedScreen
                        onClicked: power.lockWhenClosedScreen = checked
                    }
                }
            }

            Item {
                height: LingmoUI.Units.largeSpacing * 2
            }
        }
    }
}
