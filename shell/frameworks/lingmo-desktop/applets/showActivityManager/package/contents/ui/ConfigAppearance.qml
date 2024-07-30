/*
    SPDX-FileCopyrightText: 2020 Ivan Čukić <ivan.cukic at kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Controls

import org.kde.lingmoui as LingmoUI
import org.kde.lingmo.plasmoid
import org.kde.lingmo.core as LingmoCore
import org.kde.kcmutils as KCM

KCM.SimpleKCM {
    property alias cfg_showActivityIcon: radioCurrentActivityIcon.checked
    property alias cfg_showActivityName: checkShowActivityName.checked

    LingmoUI.FormLayout {
        RadioButton {
            id: radioCurrentActivityIcon

            LingmoUI.FormData.label: i18n("Icon:")

            text: i18n("Show the current activity icon")
        }

        RadioButton {
            id: radioGenericActivityIcon
            checked: !radioCurrentActivityIcon.checked
            text: i18n("Show the generic activity icon")
        }

        Item { LingmoUI.FormData.isSection: true }

        CheckBox {
            id: checkShowActivityName

            enabled: Plasmoid.formFactor !== LingmoCore.Types.Vertical

            LingmoUI.FormData.label: i18n("Title:")

            text: i18n("Show the current activity name")
        }
    }
}
