/*
 * Copyright (C) 2021 LingmoOS Team.
 *
 * Author:     Lingmo OS Team <reion@lingmoos.com>
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
    headerTitle: qsTr("Touchpad")

    Touchpad {
        id: touchpad

        Component.onCompleted: {
            accelSpeedSlider.load()
        }
    }

    Scrollable {
        anchors.fill: parent
        contentHeight: layout.implicitHeight

        ColumnLayout {
            id: layout
            anchors.fill: parent

            RoundedItem {
                GridLayout {
                    columns: 2
                    rowSpacing: LingmoUI.Units.largeSpacing * 2

                    Label {
                        text: qsTr("Enable")
                        Layout.fillWidth: true
                    }

                    Switch {
                        id: _enableSwitch
                        Layout.fillHeight: true
                        Layout.alignment: Qt.AlignRight
                        rightPadding: 0

                        Component.onCompleted: {
                            checked = touchpad.enabled
                        }

                        onCheckedChanged: touchpad.enabled = checked
                    }

                    Label {
                        visible: _enableSwitch.checked
                        text: qsTr("Tap to click")
                        Layout.fillWidth: true
                    }

                    Switch {
                        visible: _enableSwitch.checked
                        Layout.fillHeight: true
                        onCheckedChanged: touchpad.tapToClick = checked
                        Layout.alignment: Qt.AlignRight
                        rightPadding: 0

                        Component.onCompleted: {
                            checked = touchpad.tapToClick
                        }
                    }

                    Label {
                        visible: _enableSwitch.checked
                        text: qsTr("Natural scrolling")
                        Layout.fillWidth: true
                    }

                    Switch {
                        visible: _enableSwitch.checked
                        Layout.fillHeight: true
                        Layout.alignment: Qt.AlignRight
                        onCheckedChanged: touchpad.naturalScroll = checked
                        rightPadding: 0

                        Component.onCompleted: {
                            checked = touchpad.naturalScroll
                        }
                    }

                    Label {
                        visible: _enableSwitch.checked
                        text: qsTr("Pointer acceleration")
                    }

                    Slider {
                        id: accelSpeedSlider

                        visible: _enableSwitch.checked
                        Layout.fillWidth: true

                        from: 1
                        to: 11
                        stepSize: 1
                        property int accelSpeedValue: 0 // [-100, 100]

                        function load() {
                            accelSpeedValue = Math.round(touchpad.pointerAcceleration * 100)
                            // convert libinput pointer acceleration range [-1, 1] to slider range [1, 11]
                            value = Math.round(6 + touchpad.pointerAcceleration / 0.2)
                        }

                        function onAccelSpeedChanged(val) {
                            // check slider
                            if (val !== accelSpeedSlider.accelSpeedValue) {
                                accelSpeedSlider.accelSpeedValue = val
                                accelSpeedSlider.value = Math.round(6 + (val / 100) / 0.2)
                            }

                            // check libinput accelspeed
                            if ((val / 100) != touchpad.pointerAcceleration) {
                                touchpad.pointerAcceleration = val / 100
                            }
                        }

                        onMoved: {
                            if (touchpad != undefined) {
                                // convert slider range [1, 11] to accelSpeedValue range [-100, 100]
                                accelSpeedValue = Math.round(((value - 6) * 0.2) * 100)
                                onAccelSpeedChanged(accelSpeedValue)
                            }
                        }
                    }
                }
            }

            Item {
                height: LingmoUI.Units.smallSpacing
            }
        }
    }
}
