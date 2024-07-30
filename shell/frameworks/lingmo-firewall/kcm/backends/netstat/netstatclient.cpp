// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2018 Alexis Lopes Zubeta <contact@azubieta.net>
// SPDX-FileCopyrightText: 2020 Tomaz Canabrava <tcanabrava@kde.org>

#include "netstatclient.h"

#include <QStandardPaths>

/* Access to the Netstat Client thru the Connections Model */
static NetstatClient *_self = nullptr;

NetstatClient *NetstatClient::self()
{
    assert(_self);
    return _self;
}

NetstatClient::NetstatClient(QObject *parent)
    : QObject(parent)
    , m_connections(new ConnectionsModel(this))
{
    _self = this;
    mHasSS = !QStandardPaths::findExecutable(QStringLiteral("ss")).isEmpty();
}

bool NetstatClient::hasSS() const
{
    return mHasSS;
}

ConnectionsModel *NetstatClient::connectionsModel() const
{
    return m_connections;
}

void NetstatClient::setStatus(const QString &message)
{
    if (mStatus != message) {
        mStatus = message;
        Q_EMIT statusChanged(mStatus);
    }
}

QString NetstatClient::status() const
{
    return mStatus;
}
