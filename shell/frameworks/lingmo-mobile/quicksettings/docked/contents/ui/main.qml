// SPDX-FileCopyrightText: 2023 Devin Lin <devin@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick 2.15

import org.kde.lingmo.quicksetting.flashlight
import org.kde.lingmo.private.mobileshell.shellsettingsplugin as ShellSettings
import org.kde.lingmo.private.mobileshell.quicksettingsplugin as QS

QS.QuickSetting {
    text: i18n("Docked Mode")
    icon: "preferences-desktop-display-randr"
    enabled: ShellSettings.Settings.convergenceModeEnabled

    function toggle() {
        ShellSettings.Settings.convergenceModeEnabled = !ShellSettings.Settings.convergenceModeEnabled;
    }
}

