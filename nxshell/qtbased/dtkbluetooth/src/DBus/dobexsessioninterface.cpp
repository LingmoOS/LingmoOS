// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dobexsessioninterface.h"
#include "btobexdispatcher.h"
#include "dbluetoothutils.h"
#include <qdbusabstractinterface.h>
#include <qdbusextratypes.h>

DBLUETOOTH_BEGIN_NAMESPACE

DObexSessionInterface::DObexSessionInterface(const QString &path, QObject *parent)
    : QObject(parent)
{
#ifndef USE_FAKE_INTERFACE
    const auto &Service = QLatin1String(BlueZObexService);
    const auto &Connection = QDBusConnection::systemBus();
#else
    const auto &Service = QLatin1String(FakeBlueZObexService);
    const auto &Connection = QDBusConnection::sessionBus();
#endif
    const auto &Interface = QLatin1String(BlueZObexSessionInterface);
    m_inter = new DDBusInterface(Service, path, Interface, Connection, this);
#ifndef USE_FAKE_INTERFACE

    connect(&BluetoothObexDispatcher::instance(),
            &BluetoothObexDispatcher::sessionRemoved,
            this,
            [this](const QDBusObjectPath &session) {
                if (m_inter->path() == session.path())
                    removed();
            });

#endif
}

QString DObexSessionInterface::source() const
{
    return qdbus_cast<QString>(m_inter->property("Source"));
}

QString DObexSessionInterface::destination() const
{
    return qdbus_cast<QString>(m_inter->property("Destination"));
}

QString DObexSessionInterface::root() const
{
    return qdbus_cast<QString>(m_inter->property("Root"));
}

QString DObexSessionInterface::target() const
{
    return qdbus_cast<QString>(m_inter->property("Target"));
}

QString DObexSessionInterface::dBusPath() const
{
    return m_inter->path();
}

QDBusPendingReply<QString> DObexSessionInterface::getCapabilities()
{
    return m_inter->asyncCall("GetCapabilities");
}

DBLUETOOTH_END_NAMESPACE
