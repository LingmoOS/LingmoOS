// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "ddhcp6configinterface.h"
#include <QDBusArgument>
#include <QDBusMetaType>

DNETWORKMANAGER_BEGIN_NAMESPACE

DDHCP6ConfigInterface::DDHCP6ConfigInterface(const QByteArray &path, QObject *parent)
    : QObject(parent)
{
#ifdef USE_FAKE_INTERFACE
    const QString &Service = QStringLiteral("com.deepin.FakeNetworkManager");
    const QString &Interface = QStringLiteral("com.deepin.FakeNetworkManager.DHCP6Config");
    QDBusConnection Connection = QDBusConnection::sessionBus();
#else
    const QString &Service = QStringLiteral("org.freedesktop.NetworkManager");
    const QString &Interface = QStringLiteral("org.freedesktop.NetworkManager.DHCP6Config");
    QDBusConnection Connection = QDBusConnection::systemBus();
#endif
    m_inter = new DDBusInterface(Service, path, Interface, Connection, this);
    qRegisterMetaType<Config>("Config");
    qDBusRegisterMetaType<Config>();
}

Config DDHCP6ConfigInterface::options() const
{
    return qdbus_cast<Config>(m_inter->property("Options"));
}

DNETWORKMANAGER_END_NAMESPACE
