// SPDX-FileCopyrightText: 2022 Yari Polla <skilvingr@gmail.com>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick 2.15

import org.kde.lingmo.networkmanagement as LingmoNM
import org.kde.lingmo.private.mobileshell.quicksettingsplugin as QS

QS.QuickSetting {
    id: root

    LingmoNM.Handler {
        id: nmHandler
    }

    LingmoNM.WirelessStatus {
        id: wirelessStatus
    }

    text: i18n("Hotspot")
    icon: "network-wireless-hotspot"

    enabled: wirelessStatus.hotspotSSID.length !== 0
    status: enabled ? wirelessStatus.hotspotSSID : ""

    settingsCommand: "lingmo-open-settings kcm_mobile_hotspot"
    function toggle() {
        if (!enabled) {
            nmHandler.createHotspot();
        } else {
            nmHandler.stopHotspot();
        }
    }
}
