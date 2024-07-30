/*
 * SPDX-FileCopyrightText: 2016 Friedrich W. H. Kossebau <kossebau@kde.org>
 * SPDX-FileCopyrightText: 2022 Ismael Asensio <isma.af@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick
import QtQuick.Controls as QQC2

import org.kde.kirigami as Kirigami
import org.kde.plasma.private.weather
import org.kde.kcmutils as KCM

KCM.SimpleKCM {
    property alias cfg_temperatureUnit: temperatureComboBox.unit
    property alias cfg_pressureUnit: pressureComboBox.unit
    property alias cfg_speedUnit: windSpeedComboBox.unit
    property alias cfg_visibilityUnit: visibilityComboBox.unit

    Kirigami.FormLayout {
        component UnitSelector: QQC2.ComboBox {
            property int unit

            textRole: "display"
            currentIndex: model.listIndexForUnitId(unit)
            onActivated: unit = model.unitIdForListIndex(currentIndex)
        }

        UnitSelector {
            id: temperatureComboBox
            Kirigami.FormData.label: i18nc("@label:listbox", "Temperature:")
            model: TemperatureUnitListModel
        }

        UnitSelector {
            id: pressureComboBox
            Kirigami.FormData.label: i18nc("@label:listbox", "Pressure:")
            model: PressureUnitListModel
        }

        UnitSelector {
            id: windSpeedComboBox
            Kirigami.FormData.label: i18nc("@label:listbox", "Wind speed:")
            model: WindSpeedUnitListModel
        }

        UnitSelector {
            id: visibilityComboBox
            Kirigami.FormData.label: i18nc("@label:listbox", "Visibility:")
            model: VisibilityUnitListModel
        }
    }
}
