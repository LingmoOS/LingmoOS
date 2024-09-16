// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dgenericdeviceinterface.h"

DNETWORKMANAGER_BEGIN_NAMESPACE

DGenericDeviceInterface::DGenericDeviceInterface(const QByteArray &path, QObject *parent)
    : DDeviceInterface(path, parent)
{
#ifdef USE_FAKE_INTERFACE
    const QString &Service = QStringLiteral("com.deepin.FakeNetworkManager");
    const QString &Interface = QStringLiteral("com.deepin.FakeNetworkManager.Device.Generic");
    QDBusConnection Connection = QDBusConnection::sessionBus();
#else
    const QString &Service = QStringLiteral("org.freedesktop.NetworkManager");
    const QString &Interface = QStringLiteral("org.freedesktop.NetworkManager.Device.Generic");
    QDBusConnection Connection = QDBusConnection::systemBus();
#endif
    m_genericInter = new DDBusInterface(Service, path, Interface, Connection, this);
}

QString DGenericDeviceInterface::HwAddress() const
{
    return qdbus_cast<QString>(m_genericInter->property("HwAddress"));
}

QString DGenericDeviceInterface::typeDescription() const
{
    return qdbus_cast<QString>(m_genericInter->property("TypeDescription"));
}

DNETWORKMANAGER_END_NAMESPACE
