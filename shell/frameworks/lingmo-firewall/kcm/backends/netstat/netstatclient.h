// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2018 Alexis Lopes Zubeta <contact@azubieta.net>
// SPDX-FileCopyrightText: 2020 Tomaz Canabrava <tcanabrava@kde.org>

#ifndef NETSTATCLIENT_H
#define NETSTATCLIENT_H

#include <QLoggingCategory>
#include <QObject>

#include "connectionsmodel.h"
#include "netstathelper.h"

Q_DECLARE_LOGGING_CATEGORY(NetstatClientDebug)

class NetstatClient : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString status READ status WRITE setStatus NOTIFY statusChanged)
    Q_PROPERTY(ConnectionsModel *connectionsModel READ connectionsModel CONSTANT)
    Q_PROPERTY(bool hasSS READ hasSS CONSTANT)
public:
    explicit NetstatClient(QObject *parent = nullptr);
    static NetstatClient *self();
    ConnectionsModel *connectionsModel() const;

    Q_SLOT void setStatus(const QString &message);
    QString status() const;
    Q_SIGNAL void statusChanged(const QString &output);
    bool hasSS() const;

protected:
    QString mStatus;
    ConnectionsModel *const m_connections;
    NetstatHelper *m_netstatHelper = nullptr;

    /* Netstat has been deprecated for more than 20 years,
     * let's discourage distros using it as default.
     */
    int mHasSS;
};

#endif // NETSTATCLIENT_H
