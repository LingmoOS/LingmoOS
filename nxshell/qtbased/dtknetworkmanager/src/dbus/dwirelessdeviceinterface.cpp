// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dwirelessdeviceinterface.h"
#include <QDBusMetaType>

DNETWORKMANAGER_BEGIN_NAMESPACE

DWirelessDeviceInterface::DWirelessDeviceInterface(const QByteArray &path, QObject *parent)
    : DDeviceInterface(path, parent)
{
#ifdef USE_FAKE_INTERFACE
    const QString &Service = QStringLiteral("com.deepin.FakeNetworkManager");
    const QString &Interface = QStringLiteral("com.deepin.FakeNetworkManager.Device.Wireless");
    QDBusConnection Connection = QDBusConnection::sessionBus();
#else
    const QString &Service = QStringLiteral("org.freedesktop.NetworkManager");
    const QString &Interface = QStringLiteral("org.freedesktop.NetworkManager.Device.Wireless");
    QDBusConnection Connection = QDBusConnection::systemBus();
    Connection.connect(Service, path, Interface, "AccessPointAdded", this, SIGNAL(AccessPointAdded(const QDBusObjectPath &)));
    Connection.connect(Service, path, Interface, "AccessPointRemoved", this, SIGNAL(AccessPointRemoved(const QDBusObjectPath &)));
#endif
    m_wirelessInter = new DDBusInterface(Service, path, Interface, Connection, this);
    qDBusRegisterMetaType<Config>();
    qRegisterMetaType<Config>("Config");
}

QList<QDBusObjectPath> DWirelessDeviceInterface::accessPoints() const
{
    return qdbus_cast<QList<QDBusObjectPath>>(m_wirelessInter->property("AccessPoints"));
}

QString DWirelessDeviceInterface::HwAddress() const
{
    return qdbus_cast<QString>(m_wirelessInter->property("HwAddress"));
}

QString DWirelessDeviceInterface::permHwAddress() const
{
    return qdbus_cast<QString>(m_wirelessInter->property("PermHwAddress"));
}

quint32 DWirelessDeviceInterface::mode() const
{
    return qdbus_cast<quint32>(m_wirelessInter->property("Mode"));
}

quint32 DWirelessDeviceInterface::bitrate() const
{
    return qdbus_cast<quint32>(m_wirelessInter->property("Bitrate"));
}

QDBusObjectPath DWirelessDeviceInterface::activeAccessPoint() const
{
    return qdbus_cast<QDBusObjectPath>(m_wirelessInter->property("ActiveAccessPoint"));
}

quint32 DWirelessDeviceInterface::wirelessCapabilities() const
{
    return qdbus_cast<quint32>(m_wirelessInter->property("WirelessCapabilities"));
}

qint64 DWirelessDeviceInterface::lastScan() const
{
    return qdbus_cast<qint64>(m_wirelessInter->property("LastScan"));
}

QDBusPendingReply<void> DWirelessDeviceInterface::requestScan(const Config &options) const
{
    return m_wirelessInter->asyncCallWithArgumentList("RequestScan", {QVariant::fromValue(options)});
}

QDBusPendingReply<QList<QDBusObjectPath>> DWirelessDeviceInterface::getAllAccessPoints() const
{
    return m_wirelessInter->asyncCall("GetAllAccessPoints");
}

DNETWORKMANAGER_END_NAMESPACE
