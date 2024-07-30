/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * SPDX-FileCopyrightText: 2021 Ivan Podkurkov <podkiva2@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "gattserviceremote.h"
#include "gattserviceremote_p.h"
#include "device.h"
#include "device_p.h"
#include "pendingcall.h"
#include "utils.h"

namespace BluezQt
{

GattServiceRemote::GattServiceRemote(const QString &path, const QVariantMap &properties, DevicePtr adapter)
    : QObject()
    , d(new GattServiceRemotePrivate(path, properties, adapter))
{
}

GattServiceRemote::~GattServiceRemote()
{
}

GattServiceRemotePtr GattServiceRemote::toSharedPtr() const
{
    return d->q.toStrongRef();
}

QString GattServiceRemote::ubi() const
{
    return d->m_bluezGattService->path();
}

QString GattServiceRemote::uuid() const
{
    return d->m_uuid;
}

bool GattServiceRemote::isPrimary() const
{
    return d->m_primary;
}

DevicePtr GattServiceRemote::device() const
{
    return d->m_device;
}

QList<QDBusObjectPath> GattServiceRemote::includes() const
{
    return d->m_includes;
}

quint16 GattServiceRemote::handle() const
{
    return d->m_handle;
}

PendingCall* GattServiceRemote::setHandle(quint16 handle)
{
    return new PendingCall(d->setDBusProperty(QStringLiteral("Handle"), QVariant::fromValue(handle)), PendingCall::ReturnVoid, this);
}

QList<GattCharacteristicRemotePtr> GattServiceRemote::characteristics() const
{
    return d->m_characteristics;
}

} // namespace BluezQt

#include "moc_gattserviceremote.cpp"
