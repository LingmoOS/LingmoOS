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
 * A pie chart with text in the middle.
 */
Control {
    property alias valueSources: pie.valueSources
    property alias names: nameSource.array
    property alias color: colorSource.baseColor
    property alias range: pie.range
    property alias chart: pie
    property alias highlight: pie.highlight

    property alias text: centerText.text

    implicitWidth: Theme.gridUnit * 5
    implicitHeight: Theme.gridUnit * 5

    contentItem: Item {
        Charts.PieChart {
            id: pie

            anchors.fill: parent

            nameSource: Charts.ArraySource { id: nameSource; array: [ ] }
            colorSource: Charts.ColorGradientSource { id: colorSource; baseColor: Theme.highlightColor; itemCount: pie.valueSources.length }
        }

        Label {
            id: centerText

            anchors.centerIn: parent;

            horizontalAlignment: Qt.AlignHCenter
            verticalAlignment: Qt.AlignVCenter
        }
    }
}
