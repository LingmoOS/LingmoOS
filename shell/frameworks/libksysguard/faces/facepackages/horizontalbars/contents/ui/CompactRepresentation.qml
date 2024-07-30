/*
    SPDX-FileCopyrightText: 2019 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2019 David Edmundson <davidedmundson@kde.org>
    SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

import org.kde.ksysguard.sensors as Sensors
import org.kde.ksysguard.faces as Faces

import org.kde.quickcharts as Charts
import org.kde.quickcharts.controls as ChartsControls


Faces.CompactSensorFace {
    id: root

    // For vertical panels, base minimum height on the amount of items in the face
    readonly property int barCount: controller.highPrioritySensorIds.length
    readonly property real minimumTotalHeight: Kirigami.Units.smallSpacing * barCount + (barCount - 1)
    Layout.minimumHeight: verticalFormFactor ? Math.max(minimumTotalHeight, Kirigami.Units.gridUnit) : defaultMinimumSize

    contentItem: ColumnLayout {
        spacing: 1

        Item { Layout.fillWidth: true }

        Repeater {
            model: root.controller.highPrioritySensorIds

            Bar {
                Layout.fillHeight: true
                Layout.minimumHeight: Kirigami.Units.smallSpacing
                Layout.maximumHeight: Kirigami.Units.largeSpacing

                topInset: 0
                bottomInset: 0

                opacity: y + height <= root.height
                sensor: sensor
                controller: root.controller
                color: root.colorSource.map[modelData]

                Sensors.Sensor {
                    id: sensor
                    sensorId: modelData
                    updateRateLimit: root.controller.updateRateLimit
                }
            }
        }

        Item { Layout.fillWidth: true }
    }
}
