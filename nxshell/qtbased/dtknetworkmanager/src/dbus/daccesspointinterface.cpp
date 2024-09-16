// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "daccesspointinterface.h"
#include <QDBusArgument>

DNETWORKMANAGER_BEGIN_NAMESPACE

DAccessPointInterface::DAccessPointInterface(const QByteArray &path, QObject *parent)
    : QObject(parent)
{
#ifdef USE_FAKE_INTERFACE
    const QString &Service = QStringLiteral("com.deepin.FakeNetworkManager");
    const QString &Interface = QStringLiteral("com.deepin.FakeNetworkManager.AccessPoint");
    QDBusConnection Connection = QDBusConnection::sessionBus();
#else
    const QString &Service = QStringLiteral("org.freedesktop.NetworkManager");
    const QString &Interface = QStringLiteral("org.freedesktop.NetworkManager.AccessPoint");
    QDBusConnection Connection = QDBusConnection::systemBus();
#endif
    m_inter = new DDBusInterface(Service, path, Interface, Connection, this);
}

QString DAccessPointInterface::SSID() const
{
    return qdbus_cast<QString>(m_inter->property("Ssid"));
}

quint8 DAccessPointInterface::strength() const
{
    return qdbus_cast<quint8>(m_inter->property("Strength"));
}

quint32 DAccessPointInterface::frequency() const
{
    return qdbus_cast<quint32>(m_inter->property("Frequency"));
}

quint32 DAccessPointInterface::flags() const
{
    return qdbus_cast<quint32>(m_inter->property("Flags"));
}

quint32 DAccessPointInterface::rsnFlags() const
{
    return qdbus_cast<quint32>(m_inter->property("RsnFlags"));
}

quint32 DAccessPointInterface::wpaFlags() const
{
    return qdbus_cast<quint32>(m_inter->property("WpaFlags"));
}

DNETWORKMANAGER_END_NAMESPACE
