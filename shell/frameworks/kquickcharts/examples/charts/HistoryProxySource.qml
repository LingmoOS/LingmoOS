/*
 * This file is part of KQuickCharts
 * SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kquickcontrols

import org.kde.quickcharts as Charts

ChartPage {
    title: "History Proxy Source"

    ListModel {
        id: lineModel;
        dynamicRoles: true;
    }

    Timer {
        id: updateTimer
        running: true
        repeat: true
        interval: 16

        property real value

        onTriggered: {
            value = Math.max(0.0, Math.min(1.0, value + (-0.05 + Math.random() / 10)));
        }
    }

    chart: Kirigami.AbstractCard {
        anchors.centerIn: parent
        width: parent.width
        height: 400

        Charts.LineChart {
            id: chart
            anchors.fill: parent

            yRange {
                from: 0
                to: 1
                automatic: false
            }

            valueSources: Charts.HistoryProxySource {
                id: historySource
                source: Charts.SingleValueSource { value: updateTimer.value }
                maximumHistory: 100
            }

            colorSource: Charts.SingleValueSource { value: "darkRed" }

            lineWidth: 2
            fillOpacity: 0.2
        }
    }

    controls: [
        RangeEditor { label: "X Axis"; range: chart.xRange },
        RangeEditor { label: "Y Axis"; range: chart.yRange },
        RowLayout {
            Button { icon.name: "media-playback-start"; enabled: !updateTimer.running; onClicked: updateTimer.start() }
            Button { icon.name: "media-playback-stop"; enabled: updateTimer.running; onClicked: updateTimer.stop() }
            Label { text: "History Amount" }
            SpinBox {
                from: 0
                to: 99999
                stepSize: 1
                value: historySource.maximumHistory
                onValueModified: historySource.maximumHistory = value
            }
            CheckBox {
                text: "Interpolate"
                checked: chart.interpolate
                onToggled: chart.interpolate = checked
            }
            Label { text : "Interval" }
            SpinBox {
                from: 10
                to: 99999
                stepSize: 1
                value: updateTimer.interval
                onValueModified: updateTimer.interval = value
            }
        },
        RowLayout {
            Label { text: "Direction" }
            ComboBox {
                model: [
                    { value: Charts.XYChart.ZeroAtStart, text: "Zero at Start" },
                    { value: Charts.XYChart.ZeroAtEnd, text: "Zero at End" }
                ]

                textRole: "text"
                valueRole: "value"

                onActivated: chart.direction = currentValue
            }
            Label { text: "Fill Mode" }
            ComboBox {
                model: [
                    { value: Charts.HistoryProxySource.DoNotFill, text: "Do Not Fill" },
                    { value: Charts.HistoryProxySource.FillFromStart, text: "Fill from Start" },
                    { value: Charts.HistoryProxySource.FillFromEnd, text: "Fill from End" }
                ]

                textRole: "text"
                valueRole: "value"

                onActivated: historySource.fillMode = currentValue
            }
        }
    ]
}
