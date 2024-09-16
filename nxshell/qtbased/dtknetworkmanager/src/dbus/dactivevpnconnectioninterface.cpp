// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dactivevpnconnectioninterface.h"
#include <QDBusArgument>

DNETWORKMANAGER_BEGIN_NAMESPACE

DActiveVpnConnectionInterface::DActiveVpnConnectionInterface(const QByteArray &path, QObject *parent)
    : DActiveConnectionInterface(path, parent)
{
#ifdef USE_FAKE_INTERFACE
    const QString &Service = QStringLiteral("com.deepin.FakeNetworkManager");
    const QString &Interface = QStringLiteral("com.deepin.FakeNetworkManager.VPN.Connection");
    QDBusConnection Connection = QDBusConnection::sessionBus();
#else
    const QString &Service = QStringLiteral("org.freedesktop.NetworkManager");
    const QString &Interface = QStringLiteral("org.freedesktop.NetworkManager.VPN.Connection");
    QDBusConnection Connection = QDBusConnection::systemBus();
    Connection.connect(Service, path, Interface, "VpnStateChanged", this, SIGNAL(VpnStateChanged(const quint32, const quint32)));
#endif
    m_vpninter = new DDBusInterface(Service, path, Interface, Connection, this);
}

quint32 DActiveVpnConnectionInterface::vpnState() const
{
    return qdbus_cast<quint32>(m_vpninter->property("VpnState"));
};

QString DActiveVpnConnectionInterface::banner() const
{
    return qdbus_cast<QString>(m_vpninter->property("Banner"));
}

DNETWORKMANAGER_END_NAMESPACE
