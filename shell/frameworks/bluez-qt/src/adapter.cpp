/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * SPDX-FileCopyrightText: 2014 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "adapter.h"
#include "adapter_p.h"
#include "device.h"
#include "device_p.h"
#include "pendingcall.h"

namespace BluezQt
{
Adapter::Adapter(const QString &path, const QVariantMap &properties)
    : QObject()
    , d(new AdapterPrivate(path, properties))
{
}

Adapter::~Adapter() = default;

AdapterPtr Adapter::toSharedPtr() const
{
    return d->q.toStrongRef();
}

QString Adapter::ubi() const
{
    return d->m_bluezAdapter->path();
}

QString Adapter::address() const
{
    return d->m_address;
}

QString Adapter::name() const
{
    return d->m_alias;
}

PendingCall *Adapter::setName(const QString &name)
{
    return new PendingCall(d->setDBusProperty(QStringLiteral("Alias"), name), PendingCall::ReturnVoid, this);
}

QString Adapter::systemName() const
{
    return d->m_name;
}

quint32 Adapter::adapterClass() const
{
    return d->m_adapterClass;
}

bool Adapter::isPowered() const
{
    return d->m_powered;
}

PendingCall *Adapter::setPowered(bool powered)
{
    return new PendingCall(d->setDBusProperty(QStringLiteral("Powered"), powered), PendingCall::ReturnVoid, this);
}

bool Adapter::isDiscoverable() const
{
    return d->m_discoverable;
}

PendingCall *Adapter::setDiscoverable(bool discoverable)
{
    return new PendingCall(d->setDBusProperty(QStringLiteral("Discoverable"), discoverable), PendingCall::ReturnVoid, this);
}

quint32 Adapter::discoverableTimeout() const
{
    return d->m_discoverableTimeout;
}

PendingCall *Adapter::setDiscoverableTimeout(quint32 timeout)
{
    return new PendingCall(d->setDBusProperty(QStringLiteral("DiscoverableTimeout"), timeout), PendingCall::ReturnVoid, this);
}

bool Adapter::isPairable() const
{
    return d->m_pairable;
}

PendingCall *Adapter::setPairable(bool pairable)
{
    return new PendingCall(d->setDBusProperty(QStringLiteral("Pairable"), pairable), PendingCall::ReturnVoid, this);
}

quint32 Adapter::pairableTimeout() const
{
    return d->m_pairableTimeout;
}

PendingCall *Adapter::setPairableTimeout(quint32 timeout)
{
    return new PendingCall(d->setDBusProperty(QStringLiteral("PairableTimeout"), timeout), PendingCall::ReturnVoid, this);
}

bool Adapter::isDiscovering()
{
    return d->m_discovering;
}

QStringList Adapter::uuids() const
{
    return d->m_uuids;
}

QString Adapter::modalias() const
{
    return d->m_modalias;
}

GattManagerPtr Adapter::gattManager() const
{
    return d->m_gattManager;
}

LEAdvertisingManagerPtr Adapter::leAdvertisingManager() const
{
    return d->m_leAdvertisingManager;
}

MediaPtr Adapter::media() const
{
    return d->m_media;
}

QList<DevicePtr> Adapter::devices() const
{
    return d->m_devices;
}

DevicePtr Adapter::deviceForAddress(const QString &address) const
{
    for (DevicePtr device : std::as_const(d->m_devices)) {
        if (device->address() == address) {
            return device;
        }
    }
    return DevicePtr();
}

PendingCall *Adapter::startDiscovery()
{
    return new PendingCall(d->m_bluezAdapter->StartDiscovery(), PendingCall::ReturnVoid, this);
}

PendingCall *Adapter::stopDiscovery()
{
    return new PendingCall(d->m_bluezAdapter->StopDiscovery(), PendingCall::ReturnVoid, this);
}

PendingCall *Adapter::removeDevice(DevicePtr device)
{
    return new PendingCall(d->m_bluezAdapter->RemoveDevice(QDBusObjectPath(device->ubi())), PendingCall::ReturnVoid, this);
}

PendingCall* Adapter::setDiscoveryFilter(const QVariantMap& filter)
{
    return new PendingCall(d->m_bluezAdapter->SetDiscoveryFilter(filter), PendingCall::ReturnVoid, this);
}

PendingCall* Adapter::getDiscoveryFilters()
{
    return new PendingCall(d->m_bluezAdapter->GetDiscoveryFilters(), PendingCall::ReturnStringList, this);
}

} // namespace BluezQt

#include "moc_adapter.cpp"
