// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dobextransferinterface.h"
#include "btobexdispatcher.h"
#include "dbluetoothutils.h"
#include <qdbusargument.h>
#include <qdbusextratypes.h>

DBLUETOOTH_BEGIN_NAMESPACE

DObexTransferInterface::DObexTransferInterface(const QString &path, QObject *parent)
    : QObject(parent)
{
#ifndef USE_FAKE_INTERFACE
    const auto &Service = QLatin1String(BlueZObexService);
    const auto &Connection = QDBusConnection::systemBus();
#else
    const auto &Service = QLatin1String(FakeBlueZObexService);
    const auto &Connection = QDBusConnection::sessionBus();
#endif
    const auto &Interface = QLatin1String(BlueZObexTransferInterface);
    m_inter = new DDBusInterface(Service, path, Interface, Connection, this);
#ifndef USE_FAKE_INTERFACE
    connect(&BluetoothObexDispatcher::instance(),
            &BluetoothObexDispatcher::transferRemoved,
            this,
            [this](const QDBusObjectPath &transfer) {
                if (m_inter->path() == transfer.path())
                    Q_EMIT removed();
            });
#endif
}

QString DObexTransferInterface::status() const
{
    return qdbus_cast<QString>(m_inter->property("Status"));
}

QDBusObjectPath DObexTransferInterface::session() const
{
    return qdbus_cast<QDBusObjectPath>(m_inter->property("Session"));
}

QString DObexTransferInterface::name() const
{
    return qdbus_cast<QString>(m_inter->property("Name"));
}

quint64 DObexTransferInterface::size() const
{
    return qdbus_cast<quint64>(m_inter->property("Size"));
}

quint64 DObexTransferInterface::transferred() const
{
    return qdbus_cast<quint64>(m_inter->property("Transferred"));
}

QString DObexTransferInterface::filename() const
{
    return qdbus_cast<QString>(m_inter->property("Filename"));
}
QString DObexTransferInterface::type() const
{
    return qdbus_cast<QString>(m_inter->property("Type"));
}

QDBusPendingReply<void> DObexTransferInterface::cancel() const
{
    return m_inter->asyncCall("Cancel");
}

QDBusPendingReply<void> DObexTransferInterface::suspend() const
{
    return m_inter->asyncCall("Suspend");
}

QDBusPendingReply<void> DObexTransferInterface::resume() const
{
    return m_inter->asyncCall("Resume");
}

DBLUETOOTH_END_NAMESPACE
