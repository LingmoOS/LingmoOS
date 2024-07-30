/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * SPDX-FileCopyrightText: 2021 Ivan Podkurkov <podkiva2@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "gattcharacteristicremote.h"
#include "gattcharacteristicremote_p.h"
#include "pendingcall.h"
#include "utils.h"

namespace BluezQt
{

GattCharacteristicRemote::GattCharacteristicRemote(const QString &path, const QVariantMap &properties, GattServiceRemotePtr service)
    : QObject()
    , d(new GattCharacteristicRemotePrivate(path, properties, service))
{
}

GattCharacteristicRemote::~GattCharacteristicRemote()
{
}

GattCharacteristicRemotePtr GattCharacteristicRemote::toSharedPtr() const
{
    return d->q.toStrongRef();
}

QString GattCharacteristicRemote::ubi() const
{
    return d->m_bluezGattCharacteristic->path();
}

QString GattCharacteristicRemote::uuid() const
{
    return d->m_uuid;
}

QByteArray GattCharacteristicRemote::value() const
{
    return d->m_value;
}

bool GattCharacteristicRemote::isWriteAcquired() const
{
    return d->m_writeAcquired;
}

bool GattCharacteristicRemote::isNotifyAcquired() const
{
    return d->m_notifyAcquired;
}

bool GattCharacteristicRemote::isNotifying() const
{
    return d->m_notifying;
}

QStringList GattCharacteristicRemote::flags() const
{
    return d->m_flags;
}

GattServiceRemotePtr GattCharacteristicRemote::service() const
{
    return d->m_service;
}

QList<GattDescriptorRemotePtr> GattCharacteristicRemote::descriptors() const
{
    return d->m_descriptors;
}

quint16 GattCharacteristicRemote::handle() const
{
    return d->m_handle;
}

PendingCall* GattCharacteristicRemote::setHandle(quint16 handle)
{
    return new PendingCall(d->setDBusProperty(QStringLiteral("Handle"), QVariant::fromValue(handle)), PendingCall::ReturnVoid, this);
}

quint16 GattCharacteristicRemote::MTU() const
{
    return d->m_MTU;
}

PendingCall *GattCharacteristicRemote::readValue(const QVariantMap &options)
{
    return new PendingCall(d->m_bluezGattCharacteristic->ReadValue(options), PendingCall::ReturnByteArray, this);
}

PendingCall *GattCharacteristicRemote::writeValue(const QByteArray &value, const QVariantMap &options)
{
    return new PendingCall(d->m_bluezGattCharacteristic->WriteValue(value,options), PendingCall::ReturnVoid, this);
}

PendingCall *GattCharacteristicRemote::startNotify()
{
    return new PendingCall(d->m_bluezGattCharacteristic->StartNotify(), PendingCall::ReturnVoid, this);
}

PendingCall *GattCharacteristicRemote::stopNotify()
{
    return new PendingCall(d->m_bluezGattCharacteristic->StopNotify(), PendingCall::ReturnVoid, this);
}

PendingCall *GattCharacteristicRemote::confirm()
{
    return new PendingCall(d->m_bluezGattCharacteristic->Confirm(), PendingCall::ReturnVoid, this);
}

} // namespace BluezQt

#include "moc_gattcharacteristicremote.cpp"
