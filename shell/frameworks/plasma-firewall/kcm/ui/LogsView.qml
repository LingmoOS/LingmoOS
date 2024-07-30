// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2018 Alexis Lopes Zubeta <contact@azubieta.net>
// SPDX-FileCopyrightText: 2020 Tomaz Canabrava <tcanabrava@kde.org>


import QtQuick 2.12

import org.kde.kirigami 2.10 as Kirigami

import org.kcm.firewall 1.0

ViewBase {
    id: base
    property bool active: base.Kirigami.ColumnView.inViewport

    title: i18n("Firewall Logs")
    model: kcm.client.logsModel
    columns: [
        {column: LogListModel.ProtocolColumn, width: Kirigami.Units.gridUnit * 3},
        {column: LogListModel.SourceAddressColumn, width: Kirigami.Units.gridUnit * 10},
        {column: LogListModel.SourcePortColumn, width: Kirigami.Units.gridUnit * 3},
        {column: LogListModel.DestinationAddressColumn, width: Kirigami.Units.gridUnit * 10},
        {column: LogListModel.DestinationPortColumn, width: Kirigami.Units.gridUnit * 3},
        {column: LogListModel.InterfaceColumn, width: Kirigami.Units.gridUnit * 3}
    ]
    emptyListText: i18n("There are currently no firewall log entries")

    blacklistRuleFactory: kcm.client.createRuleFromLog
    blacklistColumns: [
        LogListModel.ProtocolColumn,
        LogListModel.SourceAddressColumn,
        LogListModel.SourcePortColumn,
        LogListModel.DestinationAddressColumn,
        LogListModel.DestinationPortColumn,
        LogListModel.InterfaceColumn
    ]
    blacklistRuleSuccessMessage: i18n("Created a blacklist rule from this log entry.");

    filterColumns: blacklistColumns
    onActiveChanged: kcm.client.logsAutoRefresh = active
}
