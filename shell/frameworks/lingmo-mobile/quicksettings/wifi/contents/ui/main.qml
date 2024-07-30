// SPDX-FileCopyrightText: 2022 Devin Lin <devin@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick 2.15

import org.kde.lingmo.networkmanagement as LingmoNM
import org.kde.lingmo.private.mobileshell.quicksettingsplugin as QS

QS.QuickSetting {
    LingmoNM.Handler {
        id: nmHandler
    }

    LingmoNM.EnabledConnections {
        id: enabledConnections
    }

    LingmoNM.WirelessStatus {
        id: wirelessStatus
    }

    text: i18n("Wi-Fi")
    status: enabledConnections.wirelessEnabled ? wirelessStatus.wifiSSID : ""
    icon: enabledConnections.wirelessEnabled ? "network-wireless" : "network-wireless-disconnected"
    settingsCommand: "lingmo-open-settings kcm_mobile_wifi"
    function toggle() {
        nmHandler.enableWireless(!enabledConnections.wirelessEnabled)
    }
    enabled: enabledConnections.wirelessEnabled
}
