/*
    SPDX-FileCopyrightText: 2019 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2013-2017 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

import QtQuick 2.14
import QtQuick.Layouts 1.14
import org.kde.lingmo.core 2.0 as LingmoCore
import org.kde.lingmo.components 3.0 as LingmoComponents
import org.kde.lingmo.networkmanagement as LingmoNM
import org.kde.lingmoui 2.12 as LingmoUI
import org.kde.lingmo.private.nanoshell as NanoShell

AbstractIndicator {
    id: connectionIcon

    icon.name: connectionIconProvider.connectionIcon

    LingmoComponents.BusyIndicator {
        id: connectingIndicator

        anchors.fill: parent
        running: connectionIconProvider.connecting
        visible: running
    }

    LingmoNM.NetworkStatus {
        id: networkStatus
    }

    LingmoNM.NetworkModel {
        id: connectionModel
    }

    LingmoNM.Handler {
        id: handler
    }

    LingmoNM.ConnectionIcon {
        id: connectionIconProvider
    }
    onClicked: {
        NanoShell.StartupFeedback.open(
                            connectionIconProvider.connectionIcon,
                            i18n("Network"),
                            connectionIcon.LingmoUI.ScenePosition.x + connectionIcon.width/2,
                            connectionIcon.LingmoUI.ScenePosition.y + connectionIcon.height/2,
                            Math.min(connectionIcon.width, connectionIcon.height));
        Plasmoid.executeCommand("lingmo-settings -s -m kcm_mediacenter_wifi")
    }
}
