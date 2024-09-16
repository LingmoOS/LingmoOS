// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dipv6configinterface.h"
#include <QDBusArgument>
#include <QDBusMetaType>

DNETWORKMANAGER_BEGIN_NAMESPACE

DIPv6ConfigInterface::DIPv6ConfigInterface(const QByteArray &path, QObject *parent)
    : QObject(parent)
{
#ifdef USE_FAKE_INTERFACE
    const QString &Service = QStringLiteral("com.deepin.FakeNetworkManager");
    const QString &Interface = QStringLiteral("com.deepin.FakeNetworkManager.IP6Config");
    QDBusConnection Connection = QDBusConnection::sessionBus();
#else
    const QString &Service = QStringLiteral("org.freedesktop.NetworkManager");
    const QString &Interface = QStringLiteral("org.freedesktop.NetworkManager.IP6Config");
    QDBusConnection Connection = QDBusConnection::systemBus();
#endif
    m_inter = new DDBusInterface(Service, path, Interface, Connection, this);
    qRegisterMetaType<QList<Config>>("QList<Config>");
    qDBusRegisterMetaType<QList<Config>>();
    qDBusRegisterMetaType<QList<QByteArray>>();
}

QList<Config> DIPv6ConfigInterface::addressData() const
{
    return qdbus_cast<QList<Config>>(m_inter->property("AddressData"));
}

QList<QByteArray> DIPv6ConfigInterface::nameservers() const
{
    return qdbus_cast<QList<QByteArray>>(m_inter->property("Nameservers"));
}

QString DIPv6ConfigInterface::gateway() const
{
    return qdbus_cast<QString>(m_inter->property("Gateway"));
}

DNETWORKMANAGER_END_NAMESPACE
