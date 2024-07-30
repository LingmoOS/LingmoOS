/*
    SPDX-FileCopyrightText: 2021 Arjen Hiemstra <ahiemstra@heimr.nl>
    SPDX-FileCopyrightText: 2022 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

import org.kde.ksysguard.sensors as Sensors

import org.kde.quickcharts.controls as ChartsControls

GridLayout {
    id: grid

    property QtObject controller
    property QtObject colorSource
    property int columnCount
    property int autoColumnCount

    readonly property real preferredWidth: titleMetrics.width

    readonly property bool useSensorColor: controller.faceConfiguration.useSensorColor

    columns: columnCount > 0 ? columnCount : autoColumnCount

    columnSpacing: Kirigami.Units.largeSpacing
    rowSpacing: Kirigami.Units.largeSpacing

    Kirigami.Heading {
        id: heading
        Layout.fillWidth: true
        Layout.columnSpan: parent.columns

        horizontalAlignment: Text.AlignHCenter
        elide: Text.ElideRight
        text: grid.controller.title
        visible: grid.controller.showTitle && text.length > 0
        level: 2

        TextMetrics {
            id: titleMetrics
            font: heading.font
            text: heading.text
        }
    }

    Repeater {
        model: grid.controller.highPrioritySensorIds

        SensorRect {
            Layout.fillWidth: true
            Layout.fillHeight: true
            sensor: sensor
            text: sensor.formattedValue
            sensorColor: grid.useSensorColor ? grid.colorSource.map[modelData] : Kirigami.Theme.highlightColor

            Sensors.Sensor {
                id: sensor
                sensorId: modelData
                updateRateLimit: grid.controller.updateRateLimit
            }
        }
    }
}
