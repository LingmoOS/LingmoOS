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
import Lingmo.Audio 1.0
import "../"

ItemPage {
    headerTitle: qsTr("Sound")

    function audioIcon(value) {
        if (value <= 0)
            return "audio-volume-muted-symbolic"
        else if (value <= 25)
            return "audio-volume-low-symbolic"
        else if (value <= 75)
            return "audio-volume-medium-symbolic"
        else
            return "audio-volume-high-symbolic"
    }

    function microphoneIcon(value) {
        if (value <= 25)
            return "audio-input-microphone-low-symbolic"
        else if (value <= 70)
            return "audio-input-microphone-medium-symbolic"
        else
            return "audio-input-microphone-high-symbolic"
    }

    SinkModel {
        id: paSinkModel
    }

    SourceModel {
        id: paSourceModel
    }

    PulseObjectFilterModel {
        id: paSinkFilterModel
        sortRole: "SortByDefault"
        sortOrder: Qt.DescendingOrder
        filterOutInactiveDevices: true
        sourceModel: paSinkModel
    }

    PulseObjectFilterModel {
        id: paSourceFilterModel
        sortRole: "SortByDefault"
        sortOrder: Qt.DescendingOrder
        filterOutInactiveDevices: true
        sourceModel: paSourceModel
    }

    Scrollable {
        anchors.fill: parent
        contentHeight: layout.implicitHeight

        ColumnLayout {
            id: layout
            anchors.fill: parent

            Label {
                text: qsTr("Output Devices")
                leftPadding: LingmoUI.Units.largeSpacing
                bottomPadding: LingmoUI.Units.smallSpacing
                color: LingmoUI.Theme.disabledTextColor
                visible: sinks.count >= 1
            }

            // Playback Devices
            ListView {
                id: sinks
                Layout.fillWidth: true
                interactive: false
                spacing: LingmoUI.Units.largeSpacing

                Layout.preferredHeight: {
                    var totalHeight = 0
                    for (var i = 0; i < sinks.visibleChildren.length; ++i) {
                        totalHeight += sinks.visibleChildren[i].height
                    }
                    return totalHeight
                }

                model: paSinkFilterModel

                delegate: Item {
                    width: ListView.view.width
                    height: _itemLayout.implicitHeight + LingmoUI.Units.largeSpacing * 2

                    readonly property var currentPort: Ports[ActivePortIndex]

                    Rectangle {
                        anchors.fill: parent
                        color: LingmoUI.Theme.secondBackgroundColor
                        radius: LingmoUI.Theme.bigRadius
                    }

                    ColumnLayout {
                        id: _itemLayout
                        anchors.fill: parent
                        anchors.margins: LingmoUI.Units.largeSpacing
                        spacing: LingmoUI.Units.largeSpacing

                        Label {
                            text: !currentPort ? Description : currentPort.description
                        }

                        RowLayout {
                            spacing: LingmoUI.Units.largeSpacing

                            Image {
                                Layout.leftMargin: 2
                                height: 16
                                width: height
                                sourceSize: Qt.size(width, height)
                                source: "qrc:/images/" + (LingmoUI.Theme.darkMode ? "dark" : "light") + "/" + audioIcon(slider.value / slider.to * 100) + ".svg"
                                smooth: false
                                antialiasing: true
                            }

                            Slider {
                                id: slider
                                Layout.fillWidth: true
                                value: Volume
                                from: PulseAudio.MinimalVolume
                                to: PulseAudio.NormalVolume
                                enabled: VolumeWritable
                                visible: HasVolume

                                ToolTip {
                                    parent: slider.handle
                                    visible: slider.pressed
                                    text: (slider.value / slider.to * 100).toFixed(0)
                                }

                                onMoved: {
                                    Volume = value
                                    Muted = (value === 0)
                                }
                            }
                        }
                    }
                }
            }

            Item {
                height: LingmoUI.Units.largeSpacing * 2
            }

            Label {
                text: qsTr("Input Devices")
                leftPadding: LingmoUI.Units.largeSpacing
                bottomPadding: LingmoUI.Units.smallSpacing
                color: LingmoUI.Theme.disabledTextColor
                visible: sinkInputView.count >= 1
            }

            // Recording Devices
            ListView {
                id: sinkInputView
                Layout.fillWidth: true
                Layout.preferredHeight: contentHeight

                interactive: false
                spacing: LingmoUI.Units.largeSpacing
                model: paSourceFilterModel

                delegate: Item {
                    width: ListView.view.width
                    height: _layout.implicitHeight + LingmoUI.Units.largeSpacing * 2

                    readonly property var currentPort: Ports[ActivePortIndex]

                    Rectangle {
                        anchors.fill: parent
                        color: LingmoUI.Theme.secondBackgroundColor
                        radius: LingmoUI.Theme.bigRadius
                    }

                    ColumnLayout {
                        id: _layout
                        anchors.fill: parent
                        anchors.margins: LingmoUI.Units.largeSpacing
                        spacing: LingmoUI.Units.largeSpacing

                        Label {
                            text: !currentPort ? Description : currentPort.description
                        }

                        RowLayout {
                            spacing: LingmoUI.Units.largeSpacing

                            Image {
                                Layout.leftMargin: 2
                                height: 16
                                width: height
                                sourceSize: Qt.size(width, height)
                                source: "qrc:/images/" + (LingmoUI.Theme.darkMode ? "dark" : "light") + "/" + microphoneIcon(_slider.value / _slider.to * 100) + ".svg"
                                smooth: false
                                antialiasing: true
                            }

                            Slider {
                                id: _slider
                                Layout.fillWidth: true
                                value: Volume
                                from: PulseAudio.MinimalVolume
                                to: PulseAudio.NormalVolume
                                enabled: VolumeWritable
                                visible: HasVolume

                                onMoved: {
                                    Volume = value
                                    Muted = (value === 0)
                                }

                                ToolTip {
                                    parent: _slider.handle
                                    visible: _slider.pressed
                                    text: (_slider.value / _slider.to * 100).toFixed(0)
                                }
                            }
                        }
                    }
                }
            }

            Item {
                height: LingmoUI.Units.largeSpacing * 2
            }
        }
    }
}
