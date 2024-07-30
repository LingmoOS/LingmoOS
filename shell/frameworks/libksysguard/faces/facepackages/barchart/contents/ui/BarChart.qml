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
import org.kde.ksysguard.formatter as Formatter

import org.kde.quickcharts as Charts

Charts.BarChart {
    id: chart

    readonly property int barCount: stacked ? 1 : instantiator.count

    readonly property alias sensorsModel: sensorsModel

    property int updateRateLimit

    property var controller

    orientation: controller.faceConfiguration.horizontalBars ? Charts.BarChart.HorizontalOrientation : Charts.BarChart.VerticalOrientation

    stacked: controller.faceConfiguration.barChartStacked

    spacing: Math.floor(Kirigami.Units.smallSpacing / 2)

    readonly property real rangeFromY: controller.faceConfiguration.rangeFrom *
                                       controller.faceConfiguration.rangeFromMultiplier

    readonly property real rangeToY: controller.faceConfiguration.rangeTo *
                                     controller.faceConfiguration.rangeToMultiplier

    yRange {
        from: chart.rangeFromY
        to: chart.rangeToY
        automatic: controller.faceConfiguration.rangeAuto
    }

    Sensors.SensorDataModel {
        id: sensorsModel
        sensors: controller.highPrioritySensorIds
        updateRateLimit: chart.updateRateLimit
        sensorLabels: root.controller.sensorLabels

        property int unit: (sensorsModel.ready && sensorsModel.headerData(0, Qt.Horizontal, Sensors.SensorDataModel.Unit)) ?? Formatter.Units.UnitInvalid
    }

    Instantiator {
        id: instantiator
        model: sensorsModel.sensors
        delegate: Charts.ModelSource {
            model: sensorsModel
            roleName: "Value"
            column: index
        }
        onObjectAdded: (index, object) => {
            chart.insertValueSource(index, object)
        }
        onObjectRemoved: (index, object) => {
            chart.removeValueSource(object)
        }
    }

    colorSource: root.colorSource
    nameSource: Charts.ModelSource {
        model: sensorsModel
        roleName: "Name"
        indexColumns: true
    }
    shortNameSource: Charts.ModelSource {
        roleName: "ShortName";
        model: sensorsModel
        indexColumns: true
    }
}

