/*
    SPDX-FileCopyrightText: 2019 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2019 David Edmundson <davidedmundson@kde.org>
    SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

import org.kde.ksysguard.sensors as Sensors
import org.kde.ksysguard.faces as Faces
import org.kde.ksysguard.formatter as Formatter

import org.kde.quickcharts as Charts
import org.kde.quickcharts.controls as ChartsControls


Faces.CompactSensorFace {
    id: root

    Layout.minimumWidth: horizontalFormFactor ? contentItem.Layout.minimumWidth : defaultMinimumSize
    Layout.minimumHeight: verticalFormFactor ? contentItem.Layout.minimumHeight : defaultMinimumSize
    Layout.preferredWidth: horizontalFormFactor ? contentItem.preferredWidth : -1
    Layout.maximumWidth: horizontalFormFactor ? contentItem.preferredWidth : -1

    FontMetrics {
        id: defaultMetrics
        font: Kirigami.Theme.defaultFont
    }

    FontMetrics {
        id: smallMetrics
        font: Kirigami.Theme.smallFont
    }

    readonly property bool useSmallFont: (horizontalFormFactor && height < defaultMetrics.lineSpacing + 2) || (verticalFormFactor && width < defaultMetrics.averageCharacterWidth * 6 + Kirigami.Units.smallSpacing)
    readonly property real lineHeight: useSmallFont ? smallMetrics.lineSpacing : defaultMetrics.lineSpacing

    clip: true

    contentItem: GridLayout {
        id: layout

        columnSpacing: Kirigami.Units.smallSpacing
        rowSpacing: 1

        flow: GridLayout.TopToBottom
        rows: root.verticalFormFactor ? -1 : Math.max(Math.floor(root.height / (root.lineHeight + rowSpacing)), 1)

        property real preferredWidth: {
            if (repeater.count == 0) {
                return 0
            }

            const columns = Math.ceil(repeater.count / rows)
            let width = 0
            for (let i = 0; i < columns; ++i) {
                width += repeater.itemAt(i).Layout.preferredWidth
            }
            width += (columns - 1) * columnSpacing
            return width
        }

        Repeater {
            id: repeater
            model: root.controller.highPrioritySensorIds

            ChartsControls.LegendDelegate {
                Layout.fillWidth: true
                Layout.minimumWidth: root.horizontalFormFactor ? minimumWidth : Kirigami.Units.gridUnit
                Layout.preferredWidth: root.horizontalFormFactor ? preferredWidth : minimumWidth
                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter

                name: root.controller.sensorLabels[modelData] || sensor.name
                shortName: root.controller.sensorLabels[modelData] || sensor.shortName
                value: sensor.formattedValue
                color: root.colorSource.map[modelData]

                font: root.useSmallFont ? Kirigami.Theme.smallFont : Kirigami.Theme.defaultFont

                maximumValueWidth: Formatter.Formatter.maximumLength(sensor.unit, root.useSmallFont ? Kirigami.Theme.smallFont : Kirigami.Theme.defaultFont)

                ToolTip.visible: false

                // Ugly hack:
                // When the delegate gets small enough we trigger a behaviour in text that
                // causes it to still be displayed even if its size is 0. So we need to hide
                // it when that happens. This binding should be done in LegendDelegate
                // directly but that requires a new release of Frameworks. To avoid the text
                // showing, recreate the binding here.
                // TODO Remove this when we can depend on a recent enough Frameworks.
                Component.onCompleted: contentItem.children[1].opacity = Qt.binding(() => contentItem.children[1].width > 0 ? 1 : 0)

                Sensors.Sensor {
                    id: sensor
                    sensorId: modelData
                    updateRateLimit: root.controller.updateRateLimit
                }
            }
        }
    }
}
