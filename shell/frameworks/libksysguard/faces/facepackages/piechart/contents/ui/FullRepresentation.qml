/*
    SPDX-FileCopyrightText: 2019 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2019 David Edmundson <davidedmundson@kde.org>
    SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>
    SPDX-FileCopyrightText: 2019 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

import org.kde.ksysguard.sensors as Sensors
import org.kde.ksysguard.faces as Faces

import org.kde.quickcharts as Charts
import org.kde.quickcharts.controls as ChartsControls

Faces.SensorFace {
    id: root
    readonly property bool showLegend: controller.faceConfiguration.showLegend

    Layout.minimumWidth: Kirigami.Units.gridUnit * 8
    Layout.preferredWidth: Math.max(titleMetrics.width, legend.preferredWidth)

    contentItem: ColumnLayout {
        spacing: Kirigami.Units.largeSpacing
        Kirigami.Heading {
            id: heading
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            elide: Text.ElideRight
            text: root.controller.title
            visible: root.controller.showTitle && text.length > 0
            level: 2
            TextMetrics {
                id: titleMetrics
                font: heading.font
                text: heading.text
            }
        }

        PieChart {
            id: compactRepresentation
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.minimumWidth: root.formFactor != Faces.SensorFace.Vertical ? Kirigami.Units.gridUnit * 4 : Kirigami.Units.gridUnit
            Layout.minimumHeight: root.formFactor === Faces.SensorFace.Constrained 
                ? 3 * Kirigami.Units.gridUnit
                : 5 * Kirigami.Units.gridUnit
            Layout.preferredHeight: 8 * Kirigami.Units.gridUnit
            updateRateLimit: root.controller.updateRateLimit
            highlight: legend.highlightedIndex
        }

        Faces.ExtendedLegend {
            id: legend
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.minimumHeight: implicitHeight
            visible: root.showLegend
            chart: compactRepresentation.chart
            sourceModel: root.showLegend ? compactRepresentation.sensorsModel : null
            sensorIds: root.showLegend ? root.controller.lowPrioritySensorIds : []
            updateRateLimit: root.controller.updateRateLimit
        }
    }
}
