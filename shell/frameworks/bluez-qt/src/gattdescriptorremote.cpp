/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * SPDX-FileCopyrightText: 2021 Ivan Podkurkov <podkiva2@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "gattdescriptorremote.h"
#include "gattdescriptorremote_p.h"
#include "pendingcall.h"
#include "utils.h"

namespace BluezQt {

GattDescriptorRemote::GattDescriptorRemote(const QString &path, const QVariantMap &properties, GattCharacteristicRemotePtr characteristic)
    : QObject()
    , d(new GattDescriptorRemotePrivate(path, properties, characteristic))
{
}

GattDescriptorRemote::~GattDescriptorRemote()
{
}

GattDescriptorRemotePtr GattDescriptorRemote::toSharedPtr() const
{
    return d->q.toStrongRef();
}

QString GattDescriptorRemote::ubi() const
{
    return d->m_bluezGattDescriptor->path();
}

QString GattDescriptorRemote::uuid() const
{
    return d->m_uuid;
}

QByteArray GattDescriptorRemote::value() const
{
    return d->m_value;
}

QStringList GattDescriptorRemote::flags() const
{
    return d->m_flags;
}

quint16 GattDescriptorRemote::handle() const
{
    return d->m_handle;
}

PendingCall* GattDescriptorRemote::setHandle(quint16 handle)
{
    return new PendingCall(d->setDBusProperty(QStringLiteral("Handle"), QVariant::fromValue(handle)), PendingCall::ReturnVoid, this);
}

GattCharacteristicRemotePtr GattDescriptorRemote::characteristic() const
{
    return d->m_characteristic;
}

PendingCall *GattDescriptorRemote::readValue(const QVariantMap &options)
{
    return new PendingCall(d->m_bluezGattDescriptor->ReadValue(options), PendingCall::ReturnByteArray, this);
}

PendingCall *GattDescriptorRemote::writeValue(const QByteArray &value, const QVariantMap &options)
{
    return new PendingCall(d->m_bluezGattDescriptor->WriteValue(value,options), PendingCall::ReturnVoid, this);
}

}  // namespace BluezQt

#include "moc_gattdescriptorremote.cpp"
