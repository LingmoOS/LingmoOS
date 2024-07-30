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

import org.kde.quickcharts as Charts
import org.kde.quickcharts.controls as ChartsControls

Faces.SensorFace {
    id: root

    readonly property int columnCount: root.controller.faceConfiguration.columnCount

    // When automatically determining the number of columns, use the square root
    // of the number of sensors, rounded up. This should give us a number of
    // columns that generally divides things evenly across the grid.
    readonly property int autoColumnCount: Math.ceil(Math.sqrt(controller.highPrioritySensorIds.length))

    // Arbitrary minimumWidth to make easier to align plasmoids in a predictable way
    Layout.minimumWidth: Kirigami.Units.gridUnit * 8
    Layout.preferredWidth: grid.preferredWidth + Kirigami.Units.largeSpacing

    contentItem: FaceGrid {
        id: grid

        controller: root.controller
        colorSource: root.colorSource
        columnCount: root.columnCount
        autoColumnCount: root.autoColumnCount
    }
}
