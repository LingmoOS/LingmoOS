// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include "networkinter.h"

NetworkInter::NetworkInter(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent)
    : Dtk::Core::DDBusInterface(service, path, NetworkInter::staticInterfaceName(), connection, parent)
{
}

void NetworkInter::setSync(bool sync) { }

QString NetworkInter::activeConnections()
{
    return property("ActiveConnections").value<QString>();
}

QString NetworkInter::connections()
{
    return property("Connections").value<QString>();
}

uint NetworkInter::connectivity()
{
    return property("Connectivity").value<uint>();
}

QString NetworkInter::devices()
{
    return property("Devices").value<QString>();
}

bool NetworkInter::networkingEnabled()
{
    return property("NetworkingEnabled").value<bool>();
}

void NetworkInter::setNetworkingEnabled(bool value)
{
    setProperty("NetworkingEnabled", value);
}

uint NetworkInter::state()
{
    return property("State").value<uint>();
}

bool NetworkInter::vpnEnabled()
{
    return property("VpnEnabled").value<bool>();
}

void NetworkInter::setVpnEnabled(bool value)
{
    setProperty("VpnEnabled", value);
}

QString NetworkInter::wirelessAccessPoints()
{
    return property("WirelessAccessPoints").value<QString>();
}

void NetworkInter::CallQueued(const QString &callName, const QList<QVariant> &args)
{
    asyncCallWithArgumentList(callName, args);
}
