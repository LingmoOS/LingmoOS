// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dobexagentmanagerinterface.h"
#include "dbluetoothutils.h"
#include <QVariant>

DBLUETOOTH_BEGIN_NAMESPACE

using DCORE_NAMESPACE::DDBusInterface;

DObexAgentManagerInterface::DObexAgentManagerInterface(QObject *parent)
    : QObject(parent)
{
#ifndef USE_FAKE_INTERFACE
    const auto &Connection = QDBusConnection::systemBus();
    const auto &Service = QLatin1String(BlueZObexService);
#else
    const auto &Connection = QDBusConnection::sessionBus();
    const auto &Service = QLatin1String(FakeBlueZObexService);
#endif
    m_inter = new DDBusInterface(
        Service, QLatin1String("/org/bluez/obex"), QLatin1String(BlueZObexAgentManagerInterface), Connection, this);
}

QDBusPendingReply<void> DObexAgentManagerInterface::registerAgent(const QDBusObjectPath &agent) const
{
    return m_inter->asyncCallWithArgumentList("RegisterAgent", {QVariant::fromValue(agent)});
}

QDBusPendingReply<void> DObexAgentManagerInterface::unregisterAgent(const QDBusObjectPath &agent) const
{
    return m_inter->asyncCallWithArgumentList("UnregisterAgent", {QVariant::fromValue(agent)});
}

DBLUETOOTH_END_NAMESPACE
