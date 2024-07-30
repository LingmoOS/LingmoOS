/*
 SPDX-FileCopyrigh*tText: 2019 Marco Martin <mart@kde.org>
 SPDX-FileCopyrightText: 2019 Kai Uwe Broulik <kde@broulik.de>
 SPDX-FileCopyrightText: 2024 Arjen Hiemstra <ahiemstra@heimr.nl>

 SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2

import org.kde.lingmoui as LingmoUI

import org.kde.ksysguard.sensors as Sensors
import org.kde.ksysguard.faces as Faces

LingmoUI.FormLayout {
    property alias cfg_rangeAuto: rangeAutoCheckbox.checked
    property alias cfg_rangeFrom: rangeFromSpin.value
    property alias cfg_rangeFromUnit: rangeFromSpin.unit
    property alias cfg_rangeFromMultiplier: rangeFromSpin.multiplier
    property alias cfg_rangeTo: rangeToSpin.value
    property alias cfg_rangeToUnit: rangeToSpin.unit
    property alias cfg_rangeToMultiplier: rangeToSpin.multiplier

    QQC2.CheckBox {
        id: rangeAutoCheckbox
        text: i18n("Automatic Data Range")
    }
    Faces.SensorRangeSpinBox {
        id: rangeFromSpin
        LingmoUI.FormData.label: i18n("From:")
        Layout.preferredWidth: LingmoUI.Units.gridUnit * 10
        enabled: !rangeAutoCheckbox.checked
        sensors: controller.highPrioritySensorIds
    }
    Faces.SensorRangeSpinBox {
        id: rangeToSpin
        LingmoUI.FormData.label: i18n("To:")
        Layout.preferredWidth: LingmoUI.Units.gridUnit * 10
        enabled: !rangeAutoCheckbox.checked
        sensors: controller.highPrioritySensorIds
    }
}
