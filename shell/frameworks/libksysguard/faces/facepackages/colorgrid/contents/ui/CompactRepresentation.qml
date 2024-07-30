/*
    SPDX-FileCopyrightText: 2022 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

import org.kde.ksysguard.sensors as Sensors
import org.kde.ksysguard.faces as Faces

Faces.SensorFace {
    id: root

    Layout.minimumWidth: root.formFactor == Faces.SensorFace.Vertical ? Kirigami.Units.gridUnit : Kirigami.Units.gridUnit * 2
    Layout.minimumHeight: root.formFactor == Faces.SensorFace.Vertical ? contentItem.implicitHeight : Kirigami.Units.gridUnit

    contentItem: ColumnLayout {
        spacing: Kirigami.Units.smallSpacing

        Repeater {
            model: root.controller.highPrioritySensorIds

            SensorRect {
                Layout.preferredHeight: Math.min(implicitHeight, Math.max(root.height / root.controller.highPrioritySensorIds.length - Kirigami.Units.smallSpacing * (root.controller.highPrioritySensorIds.length - 1), Kirigami.Units.smallSpacing))
                opacity: y + height <= root.height
                sensor: sensor
                text: sensor.formattedValue
                useSensorColor: root.controller.faceConfiguration.useSensorColor

                Sensors.Sensor {
                    id: sensor
                    sensorId: modelData
                    updateRateLimit: root.controller.updateRateLimit
                }
            }
        }
    }
}
