/*
 * This file is part of KQuickCharts
 * SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

import QtQuick
import QtQuick.Controls

import org.kde.quickcharts as Charts
import org.kde.quickcharts.controls

/**
 * A line chart with legend, grid and axis labels.
 */
Control {

    property alias valueSources: lineChart.valueSources
    property alias names: nameSource.array
    property alias color: colorSource.baseColor

    property alias lineWidth: lineChart.lineWidth
    property alias fillOpacity: lineChart.fillOpacity
    property alias stacked: lineChart.stacked

    property alias chart: lineChart
    property alias legend: legend
    property alias xLabels: xAxisLabels
    property alias yLabels: yAxisLabels

    property alias verticalLinesVisible: verticalLines.visible
    property alias horizontalLinesVisible: horizontalLines.visible

    property alias xRange: lineChart.xRange
    property alias yRange: lineChart.yRange

    property alias xAxisSource: xAxisLabels.source
    property alias yAxisSource: yAxisLabels.source

    property alias pointDelegate: lineChart.pointDelegate

    property alias highlightEnabled: legend.highlightEnabled

    background: Rectangle { color: Theme.backgroundColor }

    contentItem: Item {
        anchors.fill: parent;

        GridLines {
            id: horizontalLines

            anchors.fill: lineChart

            chart: lineChart

            major.frequency: 2
            major.lineWidth: 2
            major.color: Qt.rgba(0.8, 0.8, 0.8, 1.0)

            minor.frequency: 1
            minor.lineWidth: 1
            minor.color: Qt.rgba(0.8, 0.8, 0.8, 1.0)
        }

        GridLines {
            id: verticalLines

            anchors.fill: lineChart

            chart: lineChart

            direction: GridLines.Vertical;

            major.count: 1
            major.lineWidth: 2
            major.color: Qt.rgba(0.8, 0.8, 0.8, 1.0)

            minor.count: 3
            minor.lineWidth: 1
            minor.color: Qt.rgba(0.8, 0.8, 0.8, 1.0)
        }

        AxisLabels {
            id: yAxisLabels

            anchors {
                left: parent.left
                top: parent.top
                bottom: xAxisLabels.top
            }

            direction: AxisLabels.VerticalBottomTop
            delegate: Label { text: AxisLabels.label }
            source: Charts.ChartAxisSource { chart: lineChart; axis: Charts.ChartAxisSource.YAxis; itemCount: 5 }
        }

        AxisLabels {
            id: xAxisLabels

            anchors {
                left: yAxisLabels.visible ? yAxisLabels.right : parent.left
                right: parent.right
                bottom: legend.top
            }

            delegate: Label { text: AxisLabels.label }
            source: Charts.ChartAxisSource { chart: lineChart; axis: Charts.ChartAxisSource.XAxis; itemCount: 5 }
        }

        Legend {
            id: legend

            anchors {
                left: yAxisLabels.visible ? yAxisLabels.right : parent.left
                right: parent.right
                bottom: parent.bottom
                bottomMargin: Theme.smallSpacing
            }

            chart: lineChart
        }

        Charts.LineChart {
            id: lineChart
            anchors {
                top: parent.top
                left: yAxisLabels.visible ? yAxisLabels.right : parent.left
                right: parent.right
                bottom: xAxisLabels.visible ? xAxisLabels.top : legend.top
            }

            xRange.automatic: true
            yRange.automatic: true

            colorSource: Charts.ColorGradientSource { id: colorSource; baseColor: Theme.highlightColor; itemCount: lineChart.valueSources.length }
            nameSource: Charts.ArraySource { id: nameSource; array: ["1", "2", "3", "4", "5"] }

            highlight: legend.highlightedIndex
        }
    }
}
