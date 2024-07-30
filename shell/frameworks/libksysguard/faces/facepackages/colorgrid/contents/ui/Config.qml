/*
    SPDX-FileCopyrightText: 2021 Arjen Hiemstra <ahiemstra@heimr.nl>
    SPDX-FileCopyrightText: 2022 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.kde.lingmoui as LingmoUI

ColumnLayout {
    id: root

    property alias cfg_columnCount: columnCountSpin.value
    property alias cfg_useSensorColor: sensorColorCheck.checked

    LingmoUI.FormLayout {
        id: form

        CheckBox {
            id: sensorColorCheck
            LingmoUI.FormData.label: i18n("Use sensor color:")
        }

        SpinBox {
            id: columnCountSpin
            LingmoUI.FormData.label: i18n("Number of Columns:")
            editable: true
            from: 0
            to: 99999

            textFromValue: function(value, locale) {
                if (value <= 0) {
                    return i18nc("@label", "Automatic")
                }
                return value.toString()
            }

            valueFromText: function(value, locale) {
                return parseInt(value)
            }
        }
    }
}
