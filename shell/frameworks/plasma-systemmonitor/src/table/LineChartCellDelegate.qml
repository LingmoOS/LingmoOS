/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQml.Models

import org.kde.kirigami as Kirigami
import org.kde.quickcharts as Charts

BaseCellDelegate {
    id: delegate

    property real maximum: 100
    property alias valueSources: chart.valueSources

    property int _row: model.row
    property int _column: model.column

    contentItem: Item {
        anchors.fill: parent
        Charts.LineChart {
            id: chart
            anchors.fill: parent

            xRange {
                from: 0
                to: 10
                automatic: false
            }

            yRange {
                from: 0
                to: delegate.maximum
                automatic: delegate.maximum <= 0
            }

            direction: Charts.XYChart.ZeroAtEnd

            opacity: 0.5
            fillOpacity: 1
            lineWidth: 0

            colorSource: Charts.SingleValueSource { value: delegate.background.selected ? Kirigami.Theme.highlightedTextColor :  Kirigami.Theme.highlightColor }
        }

        Label {
            id: label

            anchors.centerIn: parent

            padding: Kirigami.Units.smallSpacing

            text: delegate.text
            horizontalAlignment: Text.AlignLeft
            elide: Text.ElideRight
        }
    }

    ToolTip.text: delegate.text
    ToolTip.delay: Kirigami.Units.toolTipDelay
    ToolTip.visible: delegate.hovered && label.truncated
}

