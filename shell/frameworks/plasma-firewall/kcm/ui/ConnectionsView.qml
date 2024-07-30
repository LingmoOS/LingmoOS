// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2018 Alexis Lopes Zubeta <contact@azubieta.net>
// SPDX-FileCopyrightText: 2020 Tomaz Canabrava <tcanabrava@kde.org>
// SPDX-FileCopyrightText: 2020 Kai Uwe Broulik <kde@broulik.de>

import QtQuick 2.12

import org.kde.kirigami 2.14 as Kirigami

import org.kcm.firewall 1.0

ViewBase {
    id: base
    title: i18n("Connections")

    model: netStatClient.connectionsModel
    columns: [
        {column: ConnectionsModel.ProtocolColumn, width: Kirigami.Units.gridUnit * 4},
        {column: ConnectionsModel.LocalAddressColumn, width: Kirigami.Units.gridUnit * 10},
        {column: ConnectionsModel.ForeignAddressColumn, width: Kirigami.Units.gridUnit * 10},
        {column: ConnectionsModel.StatusColumn, width: Kirigami.Units.gridUnit * 5},
        {column: ConnectionsModel.PidColumn, width: Kirigami.Units.gridUnit * 3},
        {column: ConnectionsModel.ProgramColumn, width: Kirigami.Units.gridUnit * 7}
    ]
    sortColumn: ConnectionsModel.ProgramColumn
    emptyListText: i18n("There are currently no open connections")

    blacklistRuleFactory: kcm.client.createRuleFromConnection
    blacklistColumns: [
        ConnectionsModel.ProtocolColumn,
        ConnectionsModel.LocalAddressColumn,
        ConnectionsModel.ForeignAddressColumn,
        ConnectionsModel.StatusColumn
    ]
    blacklistRuleSuccessMessage: i18n("Created a blacklist rule from this connection.");

    filterColumns: [ConnectionsModel.PidColumn, ConnectionsModel.ProgramColumn]

    NetstatClient {
        id: netStatClient
        property bool active: base.Kirigami.ColumnView.inViewport
        
        function updateRunning() {
            if (!active) {
                console.log("Stopping Connections data fetch");
                netStatClient.connectionsModel.stop();
                return;
            }

            if (!netStatClient.hasSS) {
                console.log("Netstat client without ss");
                base.errorMessage.text = i18n("could not find iproute2 or net-tools packages installed.");
                base.errorMessage.visible = true;
                return;
            }

            console.log("Start connections data fetch");
            netStatClient.connectionsModel.start();
        }
        onActiveChanged: updateRunning()

        Component.onCompleted : {
            console.log("Netstat client completed.");
            if (netStatClient.hasSS) {
                console.log("Starting netstat client");
            }
            netStatClient.updateRunning();
        }
    }
}
