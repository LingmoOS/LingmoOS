// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dobexclientinterface.h"
#include "dbluetoothutils.h"

DBLUETOOTH_BEGIN_NAMESPACE

DObexClientInterface::DObexClientInterface(QObject *parent)
    : QObject(parent)
{
#ifndef USE_FAKE_INTERFACE
    const auto &Service = QLatin1String(BlueZObexService);
    const auto &Connection = QDBusConnection::systemBus();
#else
    const auto &Service = QLatin1String(FakeBlueZObexService);
    const auto &Connection = QDBusConnection::sessionBus();
#endif
    m_inter = new DDBusInterface(Service, QLatin1String("/org/bluez/obex"), QLatin1String(BlueZObexClientInterface));
}

QDBusPendingReply<QDBusObjectPath> DObexClientInterface::createSession(const QString &destination, const QVariantMap &args) const
{
    return m_inter->asyncCallWithArgumentList("CreateSession", {QVariant::fromValue(destination), QVariant::fromValue(args)});
}

QDBusPendingReply<void> DObexClientInterface::removeSession(const QDBusObjectPath &path) const
{
    return m_inter->asyncCallWithArgumentList("RemoveSession", {QVariant::fromValue(path)});
}

DBLUETOOTH_END_NAMESPACE
