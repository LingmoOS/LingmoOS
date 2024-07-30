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

import org.kde.ksysguard.faces.private
import org.kde.ksysguard.formatter
import org.kde.ksysguard.sensors

import org.kde.quickcharts as Charts
import org.kde.quickcharts.controls as ChartsControls

/**
 * A legend control to be used in faces based on org.kde.quickcharts.Controls.Legend.
 * It handles
 * layouting and display of information depending on the available space. By default the current
 * value of a sensor and its name are displayed, if it is shrunk the shortName is displayed instead.
 *
 * @since 5.19
 */
ChartsControls.Legend {
    id: legend

    /**
     * A list of sensor ids that should be displayed in addition to those from sourceModel. Typically
     * set to lowPrioritySensorIds from FaceController
     */
    property alias sensorIds: sensorsRepeater.model
    /**
     * The SensorDataModel that should be used to retrieve data about the sensors.
     */
    property SensorDataModel sourceModel
    /**
     * @deprecated since 5.21
     * Has no effect
     */
    property var colorSource

    property int updateRateLimit

    Layout.maximumHeight: implicitHeight
    Layout.maximumWidth: parent.width

    horizontalSpacing: Kirigami.Units.gridUnit
    verticalSpacing: Kirigami.Units.smallSpacing

    maximumDelegateWidth: Kirigami.Units.gridUnit * 15

    highlightEnabled: sourceModel.sensors.length > 1

    formatValue: function(input, index) {
        if (!sourceModel) {
            return input
        }

        return Formatter.formatValueShowNull(input, sourceModel.headerData(index, Qt.Horiztonal, SensorDataModel.Unit))
    }

    Binding on model {
        when: !chart
        value: QTransposeProxyModel {
            sourceModel: legend.sourceModel
        }
    }
    Binding on valueRole {
        when: !chart
        value: "Value"
    }
    Binding on nameRole {
        when: !chart
        value: "Name"
    }
    Binding on shortNameRole {
        when: !chart
        value: "ShortName"
    }
    Binding on colorRole {
        when: !chart
        value: "Color"
    }

    maximumValueWidth: function(input, index) {
        if (!sourceModel) {
            return -1
        }

        var unit = sourceModel.headerData(index, Qt.Horiztonal, SensorDataModel.Unit)
        return Formatter.maximumLength(unit, legend.font)
    }

    Repeater {
        id: sensorsRepeater
        delegate: ChartsControls.LegendDelegate {
            name: legend.sourceModel.sensorLabels[sensor.sensorId] || sensor.name
            shortName: legend.sourceModel.sensorLabels[sensor.sensorId] || sensor.shortName
            value: sensor.formattedValue || ""

            indicator: Item { }

            maximumValueWidth: legend.maximumValueWidth(sensor.value, index)

            ChartsControls.LegendLayout.minimumWidth: minimumWidth
            ChartsControls.LegendLayout.preferredWidth: preferredWidth
            ChartsControls.LegendLayout.maximumWidth: Math.max(legend.maximumDelegateWidth, preferredWidth)

            Sensor {
                id: sensor
                sensorId: modelData
                updateRateLimit: legend.updateRateLimit
            }
        }
    }
}
