/*
    SPDX-FileCopyrightText: 2020 Konrad Materka <materka@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kcmutils as KCMUtils
import org.kde.lingmoui as LingmoUI
import org.kde.lingmo.core as LingmoCore
import org.kde.lingmo.plasmoid

KCMUtils.SimpleKCM {
    property bool cfg_scaleIconsToFit
    property int cfg_iconSpacing

    LingmoUI.FormLayout {
        Layout.fillHeight: true

        QQC2.RadioButton {
            LingmoUI.FormData.label: i18nc("The arrangement of system tray icons in the Panel", "Panel icon size:")
            enabled: !LingmoUI.Settings.tabletMode
            text: i18n("Small")
            checked: !cfg_scaleIconsToFit && !LingmoUI.Settings.tabletMode
            onToggled: cfg_scaleIconsToFit = !checked
        }
        QQC2.RadioButton {
            id: automaticRadioButton
            enabled: !LingmoUI.Settings.tabletMode
            text: Plasmoid.formFactor === LingmoCore.Types.Horizontal ? i18n("Scale with Panel height")
                                                                      : i18n("Scale with Panel width")
            checked: cfg_scaleIconsToFit || LingmoUI.Settings.tabletMode
            onToggled: cfg_scaleIconsToFit = checked
        }
        QQC2.Label {
            visible: LingmoUI.Settings.tabletMode
            text: i18n("Automatically enabled when in Touch Mode")
            textFormat: Text.PlainText
            font: LingmoUI.Theme.smallFont
        }

        Item {
            LingmoUI.FormData.isSection: true
        }

        QQC2.ComboBox {
            LingmoUI.FormData.label: i18nc("@label:listbox The spacing between system tray icons in the Panel", "Panel icon spacing:")
            model: [
                {
                    "label": i18nc("@item:inlistbox Icon spacing", "Small"),
                    "spacing": 1
                },
                {
                    "label": i18nc("@item:inlistbox Icon spacing", "Normal"),
                    "spacing": 2
                },
                {
                    "label": i18nc("@item:inlistbox Icon spacing", "Large"),
                    "spacing": 6
                }
            ]
            textRole: "label"
            enabled: !LingmoUI.Settings.tabletMode

            currentIndex: {
                if (LingmoUI.Settings.tabletMode) {
                    return 2; // Large
                }

                switch (cfg_iconSpacing) {
                    case 1: return 0; // Small
                    case 2: return 1; // Normal
                    case 6: return 2; // Large
                }
            }

            onActivated: index => {
                cfg_iconSpacing = model[currentIndex]["spacing"];
            }
        }
        QQC2.Label {
            visible: LingmoUI.Settings.tabletMode
            text: i18nc("@info:usagetip under a combobox when Touch Mode is on", "Automatically set to Large when in Touch Mode")
            textFormat: Text.PlainText
            font: LingmoUI.Theme.smallFont
        }
    }
}
