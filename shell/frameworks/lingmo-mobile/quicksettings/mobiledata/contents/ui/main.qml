// SPDX-FileCopyrightText: 2022 Devin Lin <devin@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick 2.15

import org.kde.lingmo.mm as LingmoMM
import org.kde.lingmo.private.mobileshell as MobileShell
import org.kde.lingmo.private.mobileshell.quicksettingsplugin as QS

QS.QuickSetting {
    text: i18n("Mobile Data")
    icon: "network-modem"
    status: {
        if (!LingmoMM.SignalIndicator.modemAvailable) {
            return i18n("Not Available");
        } else if (LingmoMM.SignalIndicator.needsAPNAdded) {
            return i18n("APN needs to be configured in the settings");
        } else if (LingmoMM.SignalIndicator.mobileDataSupported) {
            return enabled ? i18n("On") : i18n("Off");
        } else if (LingmoMM.SignalIndicator.simEmpty) {
            return i18n("No SIM inserted");
        } else {
            return i18n("Not Available");
        }
    }

    settingsCommand: "lingmo-open-settings kcm_cellular_network"
    enabled: LingmoMM.SignalIndicator.mobileDataEnabled

    function toggle() {
        if (LingmoMM.SignalIndicator.needsAPNAdded || !LingmoMM.SignalIndicator.mobileDataSupported) {
            // open settings if unable to toggle mobile data
            MobileShell.ShellUtil.executeCommand("lingmo-open-settings kcm_cellular_network");
        } else {
            LingmoMM.SignalIndicator.mobileDataEnabled = !LingmoMM.SignalIndicator.mobileDataEnabled;
        }
    }
}
