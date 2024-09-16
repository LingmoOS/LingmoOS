// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dwireddeviceinterface.h"
#include <QDBusMetaType>

DNETWORKMANAGER_BEGIN_NAMESPACE

DWiredDeviceInterface::DWiredDeviceInterface(const QByteArray &path, QObject *parent)
    : DDeviceInterface(path, parent)
{
#ifdef USE_FAKE_INTERFACE
    const QString &Service = QStringLiteral("com.deepin.FakeNetworkManager");
    const QString &Interface = QStringLiteral("com.deepin.FakeNetworkManager.Device.Wired");
    QDBusConnection Connection = QDBusConnection::sessionBus();
#else
    const QString &Service = QStringLiteral("org.freedesktop.NetworkManager");
    const QString &Interface = QStringLiteral("org.freedesktop.NetworkManager.Device.Wired");
    QDBusConnection Connection = QDBusConnection::systemBus();
#endif
    m_wiredInter = new DDBusInterface(Service, path, Interface, Connection, this);
}

QString DWiredDeviceInterface::HwAddress() const
{
    return qdbus_cast<QString>(m_wiredInter->property("HwAddress"));
}

QString DWiredDeviceInterface::permHwAddress() const
{
    return qdbus_cast<QString>(m_wiredInter->property("PermHwAddress"));
}

quint32 DWiredDeviceInterface::speed() const
{
    return qdbus_cast<quint32>(m_wiredInter->property("Speed"));
}

QStringList DWiredDeviceInterface::S390Subchannels() const
{
    return qdbus_cast<QStringList>(m_wiredInter->property("S390Subchannels"));
}

bool DWiredDeviceInterface::carrier() const
{
    return qdbus_cast<bool>(m_wiredInter->property("Carrier"));
}

DNETWORKMANAGER_END_NAMESPACE
