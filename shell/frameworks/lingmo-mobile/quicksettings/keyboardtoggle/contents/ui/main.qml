/*
 * SPDX-FileCopyrightText: 2021 Aleix Pol Gonzalez <aleixpol@kde.org>
 * SPDX-FileCopyrightText: 2022 Devin Lin <devin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import org.kde.lingmo.private.mobileshell.quicksettingsplugin as QS
import org.kde.lingmo.private.mobileshell as MobileShell
import org.kde.lingmo.workspace.keyboardlayout 1.0 as Keyboards

QS.QuickSetting {
    text: i18n("Virtual Keyboard")
    icon: "input-keyboard-virtual"
    status: enabled ? i18n("On") :
                      (Keyboards.KWinVirtualKeyboard.available ? i18n("Off") : i18n("Tap to open settings"))
    enabled: Keyboards.KWinVirtualKeyboard.enabled && Keyboards.KWinVirtualKeyboard.available
    settingsCommand: "lingmo-open-settings kcm_mobile_onscreenkeyboard"

    function toggle() {
        if (!Keyboards.KWinVirtualKeyboard.available) {
            // select a keyboard in the settings (none is likely set)
            MobileShell.ShellUtil.executeCommand("lingmo-open-settings kcm_virtualkeyboard");
        } else {
            Keyboards.KWinVirtualKeyboard.enabled = !enabled;
        }
    }
}


