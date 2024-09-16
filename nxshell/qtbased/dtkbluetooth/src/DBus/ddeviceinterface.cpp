// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "ddeviceinterface.h"
#include "btdispatcher.h"
#include "dbluetoothutils.h"
#include <QObject>

DBLUETOOTH_BEGIN_NAMESPACE

DDeviceInterface::DDeviceInterface(const QString &path, QObject *parent)
    : QObject(parent)
{
#ifndef USE_FAKE_INTERFACE
    const auto &Service = QLatin1String(BlueZService);
    const auto &Connection = QDBusConnection::systemBus();
#else
    const auto &Service = QLatin1String(FakeBlueZService);
    const auto &Connection = QDBusConnection::sessionBus();
#endif
    m_inter = new DDBusInterface(Service, path, QLatin1String(BlueZDeviceInterface), Connection, this);
#ifndef USE_FAKE_INTERFACE
    BluetoothDispatcher::connect(
        &BluetoothDispatcher::instance(), &BluetoothDispatcher::deviceRemoved, this, [this](const QDBusObjectPath &device) {
            if (m_inter->path() == device.path())
                Q_EMIT removed();
        });
#endif
}

bool DDeviceInterface::blocked() const
{
    return qdbus_cast<bool>(m_inter->property("Blocked"));
}

void DDeviceInterface::setBlocked(bool blocked)
{
    m_inter->setProperty("Blocked", blocked);
}

bool DDeviceInterface::connected() const
{
    return qdbus_cast<bool>(m_inter->property("Connected"));
}

bool DDeviceInterface::legacyPairing() const
{
    return qdbus_cast<bool>(m_inter->property("LegacyPairing"));
}

bool DDeviceInterface::paired() const
{
    return qdbus_cast<bool>(m_inter->property("Paired"));
}

bool DDeviceInterface::servicesResolved() const
{
    return qdbus_cast<bool>(m_inter->property("ServicesResolved"));
}

bool DDeviceInterface::trusted() const
{
    return qdbus_cast<bool>(m_inter->property("Trusted"));
}

void DDeviceInterface::setTrusted(bool trusted)
{
    m_inter->setProperty("Trusted", trusted);
}

QString DDeviceInterface::adapter() const
{
    return qdbus_cast<QString>(m_inter->property("Adapter"));
}

QString DDeviceInterface::address() const
{
    return qdbus_cast<QString>(m_inter->property("Address"));
}

QString DDeviceInterface::alias() const
{
    return qdbus_cast<QString>(m_inter->property("Alias"));
}

QStringList DDeviceInterface::UUIDs() const
{
    return qdbus_cast<QStringList>(m_inter->property("UUIDs"));
}

QString DDeviceInterface::name() const
{
    return qdbus_cast<QString>(m_inter->property("Name"));
}

QString DDeviceInterface::icon() const
{
    return qdbus_cast<QString>(m_inter->property("Icon"));
}

QString DDeviceInterface::addressType() const
{
    return qdbus_cast<QString>(m_inter->property("AddressType"));
}

quint32 DDeviceInterface::Class() const
{
    return qdbus_cast<quint32>(m_inter->property("Class"));
}

quint16 DDeviceInterface::appearance() const
{
    return qdbus_cast<quint16>(m_inter->property("Appearance"));
}

qint16 DDeviceInterface::RSSI() const
{
    return qdbus_cast<qint16>(m_inter->property("RSSI"));
}

bool DDeviceInterface::isValid() const
{
    return m_inter->isValid();
}

// Methods
QDBusPendingReply<void> DDeviceInterface::disconnect()
{
    return m_inter->asyncCall("Disconnect");
}

QDBusPendingReply<void> DDeviceInterface::cancelPairing()
{
    return m_inter->asyncCall("CancelPairing");
}

QDBusPendingReply<void> DDeviceInterface::connect()
{
    return m_inter->asyncCall("Connect");
}

QDBusPendingReply<void> DDeviceInterface::pair()
{
    return m_inter->asyncCall("Pair");
}

DBLUETOOTH_END_NAMESPACE
