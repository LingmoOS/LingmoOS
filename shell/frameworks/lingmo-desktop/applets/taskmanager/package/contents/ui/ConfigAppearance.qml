/*
    SPDX-FileCopyrightText: 2013 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import org.kde.lingmoui 2.19 as LingmoUI
import org.kde.lingmo.core as LingmoCore
import org.kde.lingmo.plasmoid 2.0
import org.kde.kcmutils as KCM

KCM.SimpleKCM {
    readonly property bool lingmoPaAvailable: Qt.createComponent("PulseAudio.qml").status === Component.Ready
    readonly property bool plasmoidVertical: Plasmoid.formFactor === LingmoCore.Types.Vertical
    readonly property bool iconOnly: Plasmoid.pluginName === "org.kde.lingmo.icontasks"

    property alias cfg_showToolTips: showToolTips.checked
    property alias cfg_highlightWindows: highlightWindows.checked
    property bool cfg_indicateAudioStreams
    property alias cfg_fill: fill.checked
    property alias cfg_maxStripes: maxStripes.value
    property alias cfg_forceStripes: forceStripes.checked
    property alias cfg_taskMaxWidth: taskMaxWidth.currentIndex
    property int cfg_iconSpacing: 0

    Component.onCompleted: {
        /* Don't rely on bindings for checking the radiobuttons
           When checking forceStripes, the condition for the checked value for the allow stripes button
           became true and that one got checked instead, stealing the checked state for the just clicked checkbox
        */
        if (maxStripes.value === 1) {
            forbidStripes.checked = true;
        } else if (!Plasmoid.configuration.forceStripes && maxStripes.value > 1) {
            allowStripes.checked = true;
        } else if (Plasmoid.configuration.forceStripes && maxStripes.value > 1) {
            forceStripes.checked = true;
        }
    }
    LingmoUI.FormLayout {
        CheckBox {
            id: showToolTips
            LingmoUI.FormData.label: i18n("General:")
            text: i18n("Show small window previews when hovering over Tasks")
        }

        CheckBox {
            id: highlightWindows
            text: i18n("Hide other windows when hovering over previews")
        }

        CheckBox {
            id: indicateAudioStreams
            text: i18n("Mark applications that play audio")
            checked: cfg_indicateAudioStreams && lingmoPaAvailable
            onToggled: cfg_indicateAudioStreams = checked
            enabled: lingmoPaAvailable
        }

        CheckBox {
            id: fill
            text: i18nc("@option:check", "Fill free space on Panel")
        }

        Item {
            LingmoUI.FormData.isSection: true
            visible: !iconOnly
        }

        ComboBox {
            id: taskMaxWidth
            visible: !iconOnly && !plasmoidVertical

            LingmoUI.FormData.label: i18n("Maximum task width:")

            model: [
                i18nc("@item:inlistbox how wide a task item should be", "Narrow"),
                i18nc("@item:inlistbox how wide a task item should be", "Medium"),
                i18nc("@item:inlistbox how wide a task item should be", "Wide")
            ]
        }

        Item {
            LingmoUI.FormData.isSection: true
        }

        RadioButton {
            id: forbidStripes
            LingmoUI.FormData.label: plasmoidVertical ? i18nc("@option: radio", "Use multi-column view:") : i18nc("@option:radio", "Use multi-row view:")
            onToggled: {
                if (checked) {
                    maxStripes.value = 1
                }
            }
            text: i18nc("Never use multi-column view for Task Manager", "Never")
        }

        RadioButton {
            id: allowStripes
            onToggled: {
                if (checked) {
                    maxStripes.value = Math.max(2, maxStripes.value)
                }
            }
            text: i18nc("When to use multi-row view in Task Manager", "When Panel is low on space and thick enough")
        }

        RadioButton {
            id: forceStripes
            onToggled: {
                if (checked) {
                    maxStripes.value = Math.max(2, maxStripes.value)
                }
            }
            text: i18nc("When to use multi-row view in Task Manager", "Always when Panel is thick enough")
        }

        SpinBox {
            id: maxStripes
            enabled: maxStripes.value > 1
            LingmoUI.FormData.label: plasmoidVertical ? i18nc("@label:spinbox", "Maximum columns:") : i18nc("@label:spinbox", "Maximum rows:")
            from: 1
        }

        Item {
            LingmoUI.FormData.isSection: true
        }

        ComboBox {
            visible: iconOnly
            LingmoUI.FormData.label: i18n("Spacing between icons:")

            model: [
                {
                    "label": i18nc("@item:inlistbox Icon spacing", "Small"),
                    "spacing": 0
                },
                {
                    "label": i18nc("@item:inlistbox Icon spacing", "Normal"),
                    "spacing": 1
                },
                {
                    "label": i18nc("@item:inlistbox Icon spacing", "Large"),
                    "spacing": 3
                },
            ]

            textRole: "label"
            enabled: !LingmoUI.Settings.tabletMode

            currentIndex: {
                if (LingmoUI.Settings.tabletMode) {
                    return 2; // Large
                }

                switch (cfg_iconSpacing) {
                    case 0: return 0; // Small
                    case 1: return 1; // Normal
                    case 3: return 2; // Large
                }
            }
            onActivated: cfg_iconSpacing = model[currentIndex]["spacing"];
        }

        Label {
            visible: LingmoUI.Settings.tabletMode
            text: i18nc("@info:usagetip under a set of radio buttons when Touch Mode is on", "Automatically set to Large when in Touch Mode")
            font: LingmoUI.Theme.smallFont
        }
    }
}
