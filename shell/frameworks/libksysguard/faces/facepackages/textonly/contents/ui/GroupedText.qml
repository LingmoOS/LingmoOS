/*
    SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.quickcharts as Charts
import org.kde.quickcharts.controls as ChartsControls
import org.kde.ksysguard.sensors as Sensors
import org.kde.ksysguard.formatter as Formatter

ColumnLayout {
    id: root

    property var totalSensorIds
    property var highPrioritySensorIds
    property var lowPrioritySensorIds
    property bool showGroups: false
    property var colorSource
    property real totalHeight
    property int updateRateLimit
    property var sensorLabels

    readonly property real contentWidth: {
        let w = 0
        for (let i in children) {
            let child = children[i]
            if (child.hasOwnProperty("preferredWidth")) {
                w = Math.max(w, child.preferredWidth)
            }
        }
        return w
    }

    readonly property bool hasGroups: showGroups && root.totalSensorIds.length > 0

    Layout.minimumHeight: hasGroups ? Kirigami.Units.gridUnit * 3 : Kirigami.Units.gridUnit

    Repeater {
        model: root.hasGroups ? root.totalSensorIds : 1

        ColumnLayout {
            property string title
            property var sensors: []
            property bool useFullName: true
            property var colorSource

            readonly property alias preferredWidth: legend.preferredWidth

            Kirigami.Heading {
                Layout.fillWidth: true
                text: groupSensor.formattedValue
                level: 3
                horizontalAlignment: Text.AlignLeft
                opacity: (root.y + parent.y + y + height) < root.totalHeight ? 1 : 0
                visible: text.length > 0
                elide: Text.ElideRight
            }

            ChartsControls.Legend {
                id: legend

                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.maximumHeight: implicitHeight
                Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter

                horizontalSpacing: Kirigami.Units.gridUnit
                verticalSpacing: Kirigami.Units.smallSpacing

                model: {
                    if (!root.hasGroups) {
                        return root.highPrioritySensors.concat(root.lowPrioritySensors)
                    }

                    let result = []
                    for (let i = 0; i < root.highPrioritySensors.length; ++i) {
                        if (root.highPrioritySensors[i].name.startsWith(groupSensor.value)) {
                            result.push(root.highPrioritySensors[i])
                        }
                    }
                    for (let i = 0; i < root.lowPrioritySensors.length; ++i) {
                        if (root.lowPrioritySensors[i].name.startsWith(groupSensor.value)) {
                            result.push(root.lowPrioritySensors[i])
                        }
                    }
                    return result
                }

                delegate: ChartsControls.LegendDelegate {
                    name: root.sensorLabels[modelData.sensorId] || (root.showGroups ? modelData.shortName : modelData.name)
                    shortName: root.sensorLabels[modelData.sensorId] || modelData.shortName
                    value: modelData.formattedValue
                    color: root.colorSource.map[modelData.sensorId]

                    maximumValueWidth: {
                        var unit = modelData.unit
                        return Formatter.Formatter.maximumLength(unit, legend.font)
                    }

                    ChartsControls.LegendLayout.minimumWidth: minimumWidth
                    ChartsControls.LegendLayout.preferredWidth: preferredWidth
                    ChartsControls.LegendLayout.maximumWidth: Math.max(preferredWidth, Kirigami.Units.gridUnit * 17)
                }
            }

            Sensors.Sensor {
                id: groupSensor
                sensorId: root.hasGroups ? modelData : ""
                updateRateLimit: root.updateRateLimit
            }
        }
    }

    property var highPrioritySensors: []
    property var lowPrioritySensors: []

    Instantiator {
        model: root.highPrioritySensorIds

        Sensors.Sensor { sensorId: modelData; updateRateLimit: root.updateRateLimit }

        onObjectAdded: (index, object) => {
            root.highPrioritySensors.push(object)
            root.highPrioritySensors = root.highPrioritySensors
        }
        onObjectRemoved: (index, object) => {
            root.highPrioritySensors.splice(root.highPrioritySensors.indexOf(object), 1)
            root.highPrioritySensors = root.highPrioritySensors
        }
    }

    Instantiator {
        model: root.lowPrioritySensorIds

        Sensors.Sensor { sensorId: modelData; updateRateLimit: root.updateRateLimit }

        onObjectAdded: (index, object) => {
            root.lowPrioritySensors.push(object)
            root.lowPrioritySensors = root.lowPrioritySensors
        }
        onObjectRemoved: (index, object) => {
            root.lowPrioritySensors.splice(root.lowPrioritySensors.indexOf(object), 1)
            root.lowPrioritySensors = root.lowPrioritySensors
        }
    }
}
