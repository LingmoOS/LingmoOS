// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dagentmanagerinterface.h"
#include "dbluetoothutils.h"

DBLUETOOTH_BEGIN_NAMESPACE

DAgentManagerInterface::DAgentManagerInterface(QObject *parent)
    : QObject(parent)
{
#ifndef USE_FAKE_INTERFACE
    const auto &Service = QLatin1String(BlueZService);
    const auto &Connection = QDBusConnection::systemBus();
#else
    const auto &Service = QLatin1String(FakeBlueZService);
    const auto &Connection = QDBusConnection::sessionBus();
#endif
    m_inter =
        new DDBusInterface(Service, QLatin1String("/org/bluez"), QLatin1String(BlueZAgentManagerInterface), Connection, this);
}

QDBusPendingReply<void> DAgentManagerInterface::registerAgent(const QDBusObjectPath &agent, const QString &cap)
{
    return m_inter->asyncCallWithArgumentList("RegisterAgent", {QVariant::fromValue(agent), QVariant::fromValue(cap)});
}

QDBusPendingReply<void> DAgentManagerInterface::unregisterAgent(const QDBusObjectPath &agent)
{
    return m_inter->asyncCallWithArgumentList("UnregisterAgent", {QVariant::fromValue(agent)});
}

QDBusPendingReply<void> DAgentManagerInterface::requestDefaultAgent(const QDBusObjectPath &agent)
{
    return m_inter->asyncCallWithArgumentList("RequestDefaultAgent", {QVariant::fromValue(agent)});
}

DBLUETOOTH_END_NAMESPACE
