/*
    SPDX-FileCopyrightText: 2019 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2019 David Edmundson <davidedmundson@kde.org>
    SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>
    SPDX-FileCopyrightText: 2019 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.lingmoui as LingmoUI

import org.kde.ksysguard.sensors as Sensors
import org.kde.ksysguard.faces as Faces

import org.kde.quickcharts as Charts
import org.kde.quickcharts.controls as ChartControls

Faces.CompactSensorFace {
    id: root

    readonly property bool horizontalOrientation: controller.faceConfiguration.horizontalBars

    readonly property real minimumBarSize: (horizontalFormFactor && !horizontalOrientation) || (verticalFormFactor && horizontalOrientation) ? LingmoUI.Units.smallSpacing : 1
    readonly property real minimumBarSpacing: Math.floor(minimumBarSize * 0.25)
    readonly property real minimumTotalSize: minimumBarSize * barChart.barCount + minimumBarSpacing * (barChart.barCount - 1)
    readonly property real preferredTotalSize: LingmoUI.Units.largeSpacing * barChart.barCount + LingmoUI.Units.largeSpacing * 0.25 * (barChart.barCount - 1)

    Layout.minimumWidth: (!horizontalFormFactor || horizontalOrientation) ? defaultMinimumSize : Math.max(minimumTotalSize, LingmoUI.Units.largeSpacing)
    Layout.minimumHeight: (!verticalFormFactor || !horizontalOrientation) ? defaultMinimumSize : Math.max(minimumTotalSize, LingmoUI.Units.largeSpacing)

    Layout.preferredWidth: {
        if (!horizontalFormFactor) {
            return -1
        }

        return horizontalOrientation ? height * goldenRatio : preferredTotalSize
    }

    Layout.preferredHeight: {
        if (!verticalFormFactor) {
            return -1
        }

        return horizontalOrientation ? preferredTotalSize : width
    }

    contentItem: ColumnLayout {
        BarChart {
            id: barChart
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.alignment: Qt.AlignCenter
            updateRateLimit: root.controller.updateRateLimit
            controller: root.controller
            spacing: Math.max(Math.floor(((root.horizontalOrientation ? height : width) / barCount) * 0.25), root.minimumBarSpacing)
        }
        QQC2.Label {
            id: label
            visible: root.formFactor == Faces.SensorFace.Planar && root.controller.showTitle && text.length > 0
            Layout.alignment: Qt.AlignTop
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            text: root.controller.title
        }
    }
}
