// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2018 Alexis Lopes Zubeta <contact@azubieta.net>
// SPDX-FileCopyrightText: 2020 Tomaz Canabrava <tcanabrava@kde.org>

#pragma once

#include <QAbstractTableModel>
#include <QTimer>

#include <QLoggingCategory>

#include "netstathelper.h"

Q_DECLARE_LOGGING_CATEGORY(ConnectionsModelDebug)

struct ConnectionsData {
    QString protocol;
    QString localAddress;
    QString foreignAddress;
    QString status;
    QString pid;
    QString program;

    bool operator==(const ConnectionsData &other) const
    {
        return other.protocol == protocol && other.localAddress == localAddress && other.foreignAddress == foreignAddress && other.status == status
            && other.pid == pid && other.program == program;
    }
};

class ConnectionsModel : public QAbstractTableModel
{
    Q_OBJECT

    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)

public:
    enum ConnectionsModelColumns { ProtocolColumn = 0, LocalAddressColumn, ForeignAddressColumn, StatusColumn, PidColumn, ProgramColumn };
    Q_ENUM(ConnectionsModelColumns)

    explicit ConnectionsModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent) const override;

    bool busy() const;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    Q_INVOKABLE void start();
    Q_INVOKABLE void stop();

Q_SIGNALS:
    void countChanged();
    void busyChanged();
    void showErrorMessage(const QString &message);

protected slots:
    void refreshConnections(const QList<QStringList> &results);

private:
    QList<ConnectionsData> m_connectionsData;
    QTimer timer;
    bool m_busy = false;
    NetstatHelper m_netstatHelper;
};
