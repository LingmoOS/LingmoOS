/*
    SPDX-FileCopyrightText: 2019 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2019 David Edmundson <davidedmundson@kde.org>
    SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>
    SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.lingmoui as LingmoUI

import org.kde.ksysguard.sensors as Sensors
import org.kde.ksysguard.faces as Faces
import org.kde.ksysguard.formatter as Formatter

import org.kde.quickcharts as Charts
import org.kde.quickcharts.controls as ChartsControls

Faces.SensorFace {
    id: root

    readonly property bool showLegend: controller.faceConfiguration.showLegend
    readonly property bool showGridLines: root.controller.faceConfiguration.showGridLines
    readonly property bool horizontalBars: root.controller.faceConfiguration.horizontalBars
    readonly property bool showYAxisLabels: root.controller.faceConfiguration.showYAxisLabels

    // Arbitrary minimumWidth to make easier to align plasmoids in a predictable way
    Layout.minimumWidth: Math.max(LingmoUI.Units.gridUnit * compactRepresentation.barCount, LingmoUI.Units.gridUnit * 8)
    Layout.preferredWidth: Math.max(titleMetrics.width, legend.preferredWidth)

    contentItem: ColumnLayout {
        spacing: LingmoUI.Units.largeSpacing

        LingmoUI.Heading {
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
        RowLayout {
            spacing: LingmoUI.Units.smallSpacing
            Layout.fillHeight: true
            Layout.topMargin: showYAxisLabels ? axisMetrics.height / 2 : 0
            Layout.bottomMargin: Layout.topMargin

            Layout.minimumHeight: root.formFactor === Faces.SensorFace.Constrained
                ? 3 * LingmoUI.Units.gridUnit
                : 5 * LingmoUI.Units.gridUnit
            Layout.preferredHeight: 8 * LingmoUI.Units.gridUnit

            ChartsControls.AxisLabels {
                id: axisLabels
                visible: showYAxisLabels
                Layout.fillHeight: true
                constrainToBounds: false
                delegate: QQC2.Label {
                    anchors.right: parent.right
                    font: LingmoUI.Theme.smallFont
                    text: Formatter.Formatter.formatValueShowNull(ChartsControls.AxisLabels.label,
                                                                  compactRepresentation.sensorsModel.unit)
                    color: LingmoUI.Theme.disabledTextColor
                }
                direction: ChartsControls.AxisLabels.VerticalBottomTop
                source: Charts.ChartAxisSource {
                    chart: compactRepresentation
                    axis: Charts.ChartAxisSource.YAxis
                    itemCount: 5
                }
                TextMetrics {
                    id: axisMetrics
                    font: LingmoUI.Theme.smallFont
                    text: Formatter.Formatter.formatValueShowNull("0",
                        compactRepresentation.sensorsModel.data(compactRepresentation.sensorsModel.index(0, 0), Sensors.SensorDataModel.Unit))
                }
            }
            BarChart {
                id: compactRepresentation
                Layout.fillWidth: true
                Layout.fillHeight: true

                updateRateLimit: root.controller.updateRateLimit

                controller: root.controller

                highlight: legend.highlightedIndex

                ChartsControls.GridLines {
                    id: horizontalLines
                    visible: showGridLines
                    direction: horizontalBars ? ChartsControls.GridLines.Horizontal : ChartsControls.GridLines.Vertical
                    anchors.fill: compactRepresentation
                    z: compactRepresentation.z - 1
                    chart: compactRepresentation

                    major.count: 3
                    major.lineWidth: 1
                    // The same color as LingmoUI.Separator
                    major.color: LingmoUI.ColorUtils.linearInterpolation(LingmoUI.Theme.backgroundColor, LingmoUI.Theme.textColor, 0.2)
                    minor.visible: false
                }
            }
        }

        Faces.ExtendedLegend {
            id: legend

            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.minimumHeight: implicitHeight
            visible: root.showLegend
            chart: compactRepresentation
            sourceModel: root.showLegend ? compactRepresentation.sensorsModel : null
            sensorIds: root.showLegend ? root.controller.lowPrioritySensorIds : []
            updateRateLimit: root.controller.updateRateLimit
        }
    }
}
