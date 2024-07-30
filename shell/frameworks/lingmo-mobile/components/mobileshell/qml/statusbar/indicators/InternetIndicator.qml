/*
    SPDX-FileCopyrightText: 2021 Devin Lin <espidev@gmail.com>
    SPDX-FileCopyrightText: 2019 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2013-2017 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.lingmo.networkmanagement as LingmoNM
import org.kde.lingmoui as LingmoUI

LingmoUI.Icon {
    id: connectionIcon

    // data

    readonly property string icon: wirelessStatus.hotspotSSID.length !== 0 ? "network-wireless-hotspot" : connectionIconProvider.connectionIcon
    readonly property bool indicatorRunning: connectionIconProvider.connecting

    readonly property var networkStatus: LingmoNM.NetworkStatus {
        id: networkStatus
    }

    readonly property var networkModel: LingmoNM.NetworkModel {
        id: connectionModel
    }

    readonly property var handler: LingmoNM.Handler {
        id: handler
    }

    readonly property var wirelessStatus: LingmoNM.WirelessStatus {
        id: wirelessStatus
    }

    readonly property var connectionIcon: LingmoNM.ConnectionIcon {
        id: connectionIconProvider
    }

    // implementation
    source: icon

    QQC2.BusyIndicator {
        id: connectingIndicator

        anchors.fill: parent
        running: connectionIcon.indicatorRunning
        visible: running
    }
}
