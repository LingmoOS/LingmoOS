// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "networkconst.h"

namespace dde {

namespace network {

Connection::Connection() = default;

Connection::~Connection() = default;

QString Connection::path()
{
    return m_data.value("Path").toString();
}

QString Connection::uuid()
{
    return m_data.value("Uuid").toString();
}

QString Connection::id()
{
    return m_data.value("Id").toString();
}

QString Connection::hwAddress()
{
    return m_data.value("HwAddress").toString();
}

QString Connection::clonedAddress()
{
    return m_data.value("ClonedAddress").toString();
}

QString Connection::ssid()
{
    return m_data.value("Ssid").toString();
}

void Connection::updateConnection(const QJsonObject &data)
{
    m_data = data;
}

// 连接具体项的基类
ControllItems::ControllItems()
    : m_connection(new Connection)
{
}

ControllItems::~ControllItems()
{
    delete m_connection;
}

Connection *ControllItems::connection() const
{
    return m_connection;
}

QString ControllItems::activeConnection() const
{
    return m_activeConnection;
}

QDateTime ControllItems::timeStamp() const
{
    return m_timeStamp;
}

void ControllItems::setConnection(const QJsonObject &jsonObj)
{
    m_connection->updateConnection(jsonObj);
}

void ControllItems::setActiveConnection(const QString &activeConnection)
{
    m_activeConnection = activeConnection;
}

void ControllItems::updateTimeStamp(QDateTime timeStamp)
{
    m_timeStamp = timeStamp;
}

QDebug operator<<(QDebug dbg, ConnectionStatus status)
{
    QDebugStateSaver stateSaver(dbg);
    switch (status) {
    case ConnectionStatus::Unknown:
        dbg.nospace() << "Unknow";
        break;
    case ConnectionStatus::Activated:
        dbg.nospace() << "Activated";
        break;
    case ConnectionStatus::Activating:
        dbg.nospace() << "Activating";
        break;
    case ConnectionStatus::Deactivated:
        dbg.nospace() << "Deactivated";
        break;
    case ConnectionStatus::Deactivating:
        dbg.nospace() << "Deactivating";
        break;
    }
    return dbg;
}

}

}
