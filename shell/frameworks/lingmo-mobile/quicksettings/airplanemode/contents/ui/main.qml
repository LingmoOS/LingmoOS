/*
 *   SPDX-FileCopyrightText: 2021 Bhushan Shah <bshah@kde.org>
 *   SPDX-FileCopyrightText: 2021 Nicolas Fella <nicolas.fella@gmx.de>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

import org.kde.lingmo.networkmanagement as LingmoNM
import org.kde.lingmo.private.mobileshell.quicksettingsplugin as QS

QS.QuickSetting {
    text: i18n("Airplane Mode")
    icon: LingmoNM.Configuration.airplaneModeEnabled ? "network-flightmode-on" : "network-flightmode-off"
    status: ""
    enabled: LingmoNM.Configuration.airplaneModeEnabled

    LingmoNM.Handler {
        id: nmHandler
    }

    function toggle() {
        nmHandler.enableAirplaneMode(!LingmoNM.Configuration.airplaneModeEnabled);
        LingmoNM.Configuration.airplaneModeEnabled = !LingmoNM.Configuration.airplaneModeEnabled;
    }
}
