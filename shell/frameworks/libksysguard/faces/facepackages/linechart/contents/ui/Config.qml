/*
    SPDX-FileCopyrightText: 2019 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2

import org.kde.kirigami as Kirigami

import org.kde.ksysguard.sensors as Sensors
import org.kde.ksysguard.faces as Faces

Kirigami.FormLayout {
    id: root

    property alias cfg_showLegend: showSensorsLegendCheckbox.checked
    property alias cfg_lineChartStacked: stackedCheckbox.checked
    property alias cfg_lineChartFillOpacity: fillOpacitySpin.value
    property alias cfg_lineChartSmooth: smoothCheckbox.checked
    property alias cfg_showGridLines: showGridLinesCheckbox.checked
    property alias cfg_showYAxisLabels: showYAxisLabelsCheckbox.checked

    property alias cfg_rangeAutoY: rangeAutoYCheckbox.checked
    property alias cfg_rangeFromY: rangeFromYSpin.value
    property alias cfg_rangeToY: rangeToYSpin.value
    property alias cfg_rangeFromYUnit: rangeFromYSpin.unit
    property alias cfg_rangeToYUnit: rangeToYSpin.unit
    property alias cfg_rangeFromYMultiplier: rangeFromYSpin.multiplier
    property alias cfg_rangeToYMultiplier: rangeToYSpin.multiplier

    property alias cfg_historyAmount: historySpin.value

    // For backward compatibility
    property real cfg_rangeAutoX
    property real cfg_rangeFromX
    property real cfg_rangeToX

    Item {
        Kirigami.FormData.label: i18n("Appearance")
        Kirigami.FormData.isSection: true
    }
    QQC2.CheckBox {
        id: showSensorsLegendCheckbox
        text: i18n("Show Sensors Legend")
    }
    QQC2.CheckBox {
        id: stackedCheckbox
        text: i18n("Stacked Charts")
    }
    QQC2.CheckBox {
        id: smoothCheckbox
        text: i18n("Smooth Lines")
    }
    QQC2.CheckBox {
        id: showGridLinesCheckbox
        text: i18n("Show Grid Lines")
    }
    QQC2.CheckBox {
        id: showYAxisLabelsCheckbox
        text: i18n("Show Y Axis Labels")
    }
    QQC2.SpinBox {
        id: fillOpacitySpin
        Kirigami.FormData.label: i18n("Fill Opacity:")
        editable: true
        from: 0
        to: 100
    }
    Item {
        Kirigami.FormData.label: i18n("Data Ranges")
        Kirigami.FormData.isSection: true
    }
    QQC2.CheckBox {
        id: rangeAutoYCheckbox
        text: i18n("Automatic Y Data Range")
    }
    Faces.SensorRangeSpinBox {
        id: rangeFromYSpin
        Kirigami.FormData.label: i18n("From (Y):")
        Layout.preferredWidth: Kirigami.Units.gridUnit * 10
        enabled: !rangeAutoYCheckbox.checked
        sensors: controller.highPrioritySensorIds
    }
    Faces.SensorRangeSpinBox {
        id: rangeToYSpin
        Kirigami.FormData.label: i18n("To (Y):")
        Layout.preferredWidth: Kirigami.Units.gridUnit * 10
        enabled: !rangeAutoYCheckbox.checked
        sensors: controller.highPrioritySensorIds
    }
    QQC2.SpinBox {
        id: historySpin
        editable: true
        from: 0
        to: Math.pow(2, 31) - 1
        Kirigami.FormData.label: i18n("Amount of History to Keep:")

        textFromValue: function(value, locale) {
            return i18ncp("%1 is seconds of history", "%1 second", "%1 seconds", Number(value).toLocaleString(locale, "f", 0));
        }
        valueFromText: function(value, locale) {
            // Don't use fromLocaleString here since it will error out on extra
            // characters like the (potentially translated) seconds that gets
            // added above. Instead parseInt ignores non-numeric characters.
            return parseInt(value)
        }
    }
}

