// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: 2011 Craig Drummond <craig.p.drummond@gmail.com>
// SPDX-FileCopyrightText: 2018 Alexis Lopes Zubeta <contact@azubieta.net>
// SPDX-FileCopyrightText: 2020 Tomaz Canabrava <tcanabrava@kde.org>
/*
 * UFW KControl Module
 */

#include "loglistmodel.h"

#include <QDateTime>
#include <QRegularExpression>
#include <klocalizedstring.h>

LogListModel::LogListModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

bool LogListModel::busy() const
{
    return m_busy;
}

void LogListModel::setBusy(bool busy)
{
    if (m_busy != busy) {
        m_busy = busy;
        Q_EMIT busyChanged();
    }
}

int LogListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return m_logsData.size();
}

int LogListModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 9;
}

QVariant LogListModel::data(const QModelIndex &index, int role) const
{
    Q_UNUSED(role)
    const auto checkIndexFlags = QAbstractItemModel::CheckIndexOption::IndexIsValid | QAbstractItemModel::CheckIndexOption::ParentIsInvalid;

    if (!checkIndex(index, checkIndexFlags)) {
        return {};
    }

    LogData data = m_logsData.at(index.row());

    switch (index.column()) {
    case SourceAddressColumn:
        return data.sourceAddress;
    case SourcePortColumn:
        return data.sourcePort;
    case DestinationAddressColumn:
        return data.destinationAddress;
    case DestinationPortColumn:
        return data.destinationPort;
    case ProtocolColumn:
        return data.protocol;
    case InterfaceColumn:
        return data.interface;
    case ActionColumn:
        return data.action;
    case TimeColumn:
        return data.time;
    case DateColumn:
        return data.date;
    }
    return QVariant();
}

QVariant LogListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(orientation)
    Q_UNUSED(role)
    switch (section) {
    case SourceAddressColumn:
        return i18nc("@title:column", "From");
    case SourcePortColumn:
        return i18nc("@title:column", "Source port");
    case DestinationAddressColumn:
        return i18nc("@title:column", "To");
    case DestinationPortColumn:
        return i18nc("@title:column", "Destination port");
    case ProtocolColumn:
        return i18nc("@title:column", "Protocol");
    case InterfaceColumn:
        return i18nc("@title:column", "Interface");
    case ActionColumn:
        return i18nc("@title:column", "Action");
    case TimeColumn:
        return i18nc("@title:column", "Time");
    case DateColumn:
        return i18nc("@title:column", "Date");
    }
    return QVariant();
}

void LogListModel::appendLogData(const QList<LogData> &newData)
{
    if (newData.isEmpty()) {
        return;
    }
    beginResetModel();
    m_logsData = newData;
    endResetModel();
    Q_EMIT countChanged();
}
