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
import org.kde.ksysguard.formatter as Formatter
import org.kde.quickcharts as Charts

Charts.LineChart {
    id: chart
    
    property var controller

    readonly property alias sensorsModel: sensorsModel
    readonly property int historyAmount: controller.faceConfiguration.historyAmount

    direction: Charts.XYChart.ZeroAtEnd

    fillOpacity: controller.faceConfiguration.lineChartFillOpacity / 100
    stacked: controller.faceConfiguration.lineChartStacked
    interpolate: controller.faceConfiguration.lineChartSmooth

    //TODO: Have a central heading here too?
    //TODO: Have a plasmoid config value for line thickness?

    readonly property bool stackedAuto: chart.controller.faceConfiguration.rangeAutoY && chart.controller.faceConfiguration.lineChartStacked

    readonly property real rangeFromY: chart.controller.faceConfiguration.rangeFromY *
                                       chart.controller.faceConfiguration.rangeFromYMultiplier

    readonly property real rangeToY: chart.controller.faceConfiguration.rangeToY *
                                     chart.controller.faceConfiguration.rangeToYMultiplier

    yRange {
        from: stackedAuto ? Math.min(sensorsModel.minimum, 0) : chart.rangeFromY
        to: stackedAuto ? sensorsModel.stackedMaximum : chart.rangeToY
        automatic: (chart.controller.faceConfiguration.rangeAutoY && !chart.controller.faceConfiguration.lineChartStacked)
            || stackedAuto && yRange.from == yRange.to
    }

    Sensors.SensorDataModel {
        id: sensorsModel
        sensors: chart.controller.highPrioritySensorIds
        updateRateLimit: chart.controller.updateRateLimit
        sensorLabels: chart.controller.sensorLabels

        property int unit: (sensorsModel.ready && sensorsModel.headerData(0, Qt.Horizontal, Sensors.SensorDataModel.Unit)) ?? Formatter.Units.UnitInvalid
        property double stackedMaximum: yRange.stackedAuto ? calcStackedMaximum() : 0

        function calcStackedMaximum() {
            let max = 0
            for (let i = 0; i < sensorsModel.sensors.length; ++i) {
                max += sensorsModel.headerData(i, Qt.Horizontal, Sensors.SensorDataModel.Maximum)
            }
            return max
        }
    }

    Connections {
        target: sensorsModel
        enabled: chart.stackedAuto
        function onColumnsInserted() {
            sensorsModel.stackedMaximum = sensorsModel.calcStackedMaximum()
        }
        function onColumnsRemoved() {
            sensorsModel.stackedMaximum = sensorsModel.calcStackedMaximum()
        }
        function onSensorMetaDataChanged() {
            sensorsModel.stackedMaximum = sensorsModel.calcStackedMaximum()
        }
    }

    Instantiator {
        model: sensorsModel.sensors
        delegate: Charts.HistoryProxySource {
            id: history

            source: Charts.ModelSource {
                model: sensorsModel
                column: index
                roleName: "Value"
            }

            interval: {
                if (chart.controller.updateRateLimit > 0) {
                    return chart.controller.updateRateLimit
                }

                if (sensorsModel.ready) {
                    return sensorsModel.headerData(index, Qt.Horizontal, Sensors.SensorDataModel.UpdateInterval) ?? 0
                }

                return 0
            }
            maximumHistory: interval > 0 ? (chart.historyAmount * 1000) / interval : 0
            fillMode: Charts.HistoryProxySource.FillFromEnd

            property var connection: Connections {
                target: chart.controller
                function onUpdateRateLimitChanged() {
                    history.clear()
                }
            }
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
        roleName: "Name";
        model: sensorsModel
        indexColumns: true
    }
    shortNameSource: Charts.ModelSource {
        roleName: "ShortName";
        model: sensorsModel
        indexColumns: true
    }
}

