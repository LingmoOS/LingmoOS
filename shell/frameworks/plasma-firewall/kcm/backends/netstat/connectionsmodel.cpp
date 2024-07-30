// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2018 Alexis Lopes Zubeta <contact@azubieta.net>
// SPDX-FileCopyrightText: 2020 Tomaz Canabrava <tcanabrava@kde.org>

#include "connectionsmodel.h"

#include <chrono>

#include <QDebug>
#include <QMetaEnum>

#include <KLocalizedString>

using namespace std::chrono_literals;

ConnectionsModel::ConnectionsModel(QObject *parent)
    : QAbstractTableModel(parent)
{
    connect(&m_netstatHelper, &NetstatHelper::queryFinished, this, &ConnectionsModel::refreshConnections);
    connect(&timer, &QTimer::timeout, &m_netstatHelper, &NetstatHelper::query);
    timer.setInterval(10s); // arbitrary query interval, should be long enough to let the helper finish on its own
}

void ConnectionsModel::start()
{
    m_busy = true;
    Q_EMIT busyChanged();
    timer.start();
    QTimer::singleShot(0, &m_netstatHelper, &NetstatHelper::query);
}

void ConnectionsModel::stop()
{
    timer.stop();
}

int ConnectionsModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return m_connectionsData.size();
}

int ConnectionsModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return QMetaEnum::fromType<ConnectionsModelColumns>().keyCount();
}

bool ConnectionsModel::busy() const
{
    return m_busy;
}

QVariant ConnectionsModel::data(const QModelIndex &index, int role) const
{
    Q_UNUSED(role);
    const auto checkIndexFlags = QAbstractItemModel::CheckIndexOption::IndexIsValid | QAbstractItemModel::CheckIndexOption::ParentIsInvalid;

    if (!checkIndex(index, checkIndexFlags)) {
        return {};
    }

    ConnectionsData data = m_connectionsData.at(index.row());

    switch (static_cast<ConnectionsModelColumns>(index.column())) {
    case ProtocolColumn:
        return data.protocol;
    case LocalAddressColumn:
        return data.localAddress;
    case ForeignAddressColumn:
        return data.foreignAddress;
    case StatusColumn:
        return data.status;
    case PidColumn:
        return data.pid;
    case ProgramColumn:
        // HACK. Firefox reports as MainThread
        if (data.program == QLatin1String("MainThread")) {
            return "Firefox";
        } else {
            return data.program;
        }
        break;
    }
    return {};
}

QVariant ConnectionsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(orientation)
    Q_UNUSED(role)
    switch (static_cast<ConnectionsModelColumns>(section)) {
    case ProtocolColumn:
        return i18nc("@title:column", "Protocol");
    case LocalAddressColumn:
        return i18nc("@title:column", "Local address");
    case ForeignAddressColumn:
        return i18nc("@title:column", "Foreign address");
    case StatusColumn:
        return i18nc("@title:column", "Status");
    case PidColumn:
        return i18nc("@title:column", "PID");
    case ProgramColumn:
        return i18nc("@title:column", "Program");
    }
    return {};
}

void ConnectionsModel::refreshConnections(const QList<QStringList> &result)
{
    if (m_netstatHelper.hasError()) {
        Q_EMIT showErrorMessage(i18n("Failed to get connections: %1", m_netstatHelper.errorString()));
        return;
    }

    const auto oldConnectionsData = m_connectionsData;
    QList<ConnectionsData> newConnectionsData;

    beginResetModel();
    m_connectionsData.clear();
    for (const auto &connection : result) {
        ConnectionsData conn{.protocol = connection.at(0),
                             .localAddress = connection.at(1),
                             .foreignAddress = connection.at(2),
                             .status = connection.at(3),
                             .pid = connection.at(4),
                             .program = connection.at(5)};

        if (conn.status == QLatin1String("UNCONN")) {
            conn.status = i18n("Not Connected");
        } else if (conn.status == QLatin1String("ESTAB")) {
            conn.status = i18n("Established");
        } else if (conn.status == QLatin1String("LISTEN")) {
            conn.status = i18n("Listening");
        }

        newConnectionsData.append(conn);
    }

    if (newConnectionsData != oldConnectionsData) {
        beginResetModel();
        m_connectionsData = newConnectionsData;
        endResetModel();
    }

    if (newConnectionsData.count() != oldConnectionsData.count()) {
        Q_EMIT countChanged();
    }

    if (m_busy) {
        m_busy = false;
        Q_EMIT busyChanged();
    }
}
