// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dtundeviceinterface.h"

DNETWORKMANAGER_BEGIN_NAMESPACE

DTunDeviceInterface::DTunDeviceInterface(const QByteArray &path, QObject *parent)
    : DDeviceInterface(path, parent)
{
#ifdef USE_FAKE_INTERFACE
    const QString &Service = QStringLiteral("com.deepin.FakeNetworkManager");
    const QString &Interface = QStringLiteral("com.deepin.FakeNetworkManager.Device.Tun");
    QDBusConnection Connection = QDBusConnection::sessionBus();
#else
    const QString &Service = QStringLiteral("org.freedesktop.NetworkManager");
    const QString &Interface = QStringLiteral("org.freedesktop.NetworkManager.Device.Tun");
    QDBusConnection Connection = QDBusConnection::systemBus();
#endif
    m_tunInter = new DDBusInterface(Service, path, Interface, Connection, this);
}

QString DTunDeviceInterface::HwAddress() const
{
    return qdbus_cast<QString>(m_tunInter->property("HwAddress"));
}

DNETWORKMANAGER_END_NAMESPACE
