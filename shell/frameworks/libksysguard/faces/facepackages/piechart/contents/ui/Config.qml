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
    property alias cfg_fromAngle: fromAngleSpin.value
    property alias cfg_toAngle: toAngleSpin.value
    property alias cfg_smoothEnds: smoothEndsCheckbox.checked

    property alias cfg_rangeAuto: rangeAutoCheckbox.checked
    property alias cfg_rangeFrom: rangeFromSpin.value
    property alias cfg_rangeFromUnit: rangeFromSpin.unit
    property alias cfg_rangeFromMultiplier: rangeFromSpin.multiplier
    property alias cfg_rangeTo: rangeToSpin.value
    property alias cfg_rangeToUnit: rangeToSpin.unit
    property alias cfg_rangeToMultiplier: rangeToSpin.multiplier

    QQC2.CheckBox {
        id: showSensorsLegendCheckbox
        text: i18n("Show Sensors Legend")
    }
    QQC2.SpinBox {
        id: fromAngleSpin
        Kirigami.FormData.label: i18n("Start from Angle:")
        from: -180
        to: 360
        editable: true
        textFromValue: function(value, locale) {
            return i18nc("angle degrees", "%1°", value);
        }
        valueFromText: function(text, locale) {
            return Number.fromLocaleString(locale, text.replace(i18nc("angle degrees", "°"), ""));
        }
    }
    QQC2.SpinBox {
        id: toAngleSpin
        Kirigami.FormData.label: i18n("Total Pie Angle:")
        from: 0
        to: 360
        editable: true
        textFromValue: function(value, locale) {
            return i18nc("angle", "%1°", value);
        }
        valueFromText: function(text, locale) {
            return Number.fromLocaleString(locale, text.replace(i18nc("angle degrees", "°"), ""));
        }
    }
    QQC2.CheckBox {
        id: smoothEndsCheckbox
        text: i18n("Rounded Lines")
    }

    QQC2.CheckBox {
        id: rangeAutoCheckbox
        text: i18n("Automatic Data Range")
    }
    Faces.SensorRangeSpinBox {
        id: rangeFromSpin
        Kirigami.FormData.label: i18n("From:")
        Layout.preferredWidth: Kirigami.Units.gridUnit * 10
        enabled: !rangeAutoCheckbox.checked
        sensors: controller.highPrioritySensorIds
    }
    Faces.SensorRangeSpinBox {
        id: rangeToSpin
        Kirigami.FormData.label: i18n("To:")
        Layout.preferredWidth: Kirigami.Units.gridUnit * 10
        enabled: !rangeAutoCheckbox.checked
        sensors: controller.highPrioritySensorIds
    }
}
