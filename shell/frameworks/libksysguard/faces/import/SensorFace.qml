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
import org.kde.quickcharts as Charts
import org.kde.ksysguard.sensors as Sensors
import org.kde.ksysguard.faces as Faces

Faces.AbstractSensorFace {
    id: root

    property list<Kirigami.Action> actions

    implicitWidth: contentItem.implicitWidth
    implicitHeight: contentItem.implicitHeight
    Layout.minimumWidth: contentItem.Layout.minimumWidth
    Layout.minimumHeight: contentItem.Layout.minimumHeight
    Layout.preferredWidth: contentItem.Layout.preferredWidth
    Layout.preferredHeight: contentItem.Layout.preferredHeight
    Layout.maximumWidth: contentItem.Layout.maximumWidth
    Layout.maximumHeight: contentItem.Layout.maximumHeight

    property alias colorSource: colorSource

    Charts.MapProxySource {
        id: colorSource
        source: Charts.ArraySource {
            array: root.controller.highPrioritySensorIds
        }
        map: root.controller.sensorColors
    }
    Charts.ColorGradientSource {
        baseColor: Kirigami.Theme.highlightColor
        itemCount: root.controller.highPrioritySensorIds.length

        onItemCountChanged: generate()
        Component.onCompleted: generate()

        function generate() {
            //var colors = colorSource.colors;
            var savedColors = root.controller.sensorColors;
            for (var i = 0; i < root.controller.highPrioritySensorIds.length; ++i) {
                if (!savedColors.hasOwnProperty(root.controller.highPrioritySensorIds[i])) {
                    savedColors[root.controller.highPrioritySensorIds[i]] = colors[i];
                } else {
                    // Use the darker trick to make Qt validate the scring as a valid color;
                    var currentColor = Qt.darker(savedColors[root.controller.highPrioritySensorIds[i]], 1);
                    if (!currentColor) {
                        savedColors[root.controller.highPrioritySensorIds[i]] = colors[i];
                    } else {
                        savedColors[root.controller.highPrioritySensorIds[i]] = currentColor;
                    }
                }
            }
            root.controller.sensorColors = savedColors;
        }
    }
}
