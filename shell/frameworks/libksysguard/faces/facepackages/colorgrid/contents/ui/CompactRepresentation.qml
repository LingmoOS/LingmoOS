/*
    SPDX-FileCopyrightText: 2022 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts

import org.kde.lingmoui as LingmoUI

import org.kde.ksysguard.sensors as Sensors
import org.kde.ksysguard.faces as Faces

Faces.SensorFace {
    id: root

    Layout.minimumWidth: root.formFactor == Faces.SensorFace.Vertical ? LingmoUI.Units.gridUnit : LingmoUI.Units.gridUnit * 2
    Layout.minimumHeight: root.formFactor == Faces.SensorFace.Vertical ? contentItem.implicitHeight : LingmoUI.Units.gridUnit

    contentItem: ColumnLayout {
        spacing: LingmoUI.Units.smallSpacing

        Repeater {
            model: root.controller.highPrioritySensorIds

            SensorRect {
                Layout.preferredHeight: Math.min(implicitHeight, Math.max(root.height / root.controller.highPrioritySensorIds.length - LingmoUI.Units.smallSpacing * (root.controller.highPrioritySensorIds.length - 1), LingmoUI.Units.smallSpacing))
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
