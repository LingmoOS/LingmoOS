/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * SPDX-FileCopyrightText: 2021 Ivan Podkurkov <podkiva2@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "gattcharacteristicremote_p.h"
#include "gattcharacteristicremote.h"
#include "gattdescriptorremote.h"
#include "gattdescriptorremote_p.h"
#include "device_p.h"
#include "device.h"
#include "adapter.h"
#include "input.h"
#include "input_p.h"
#include "mediaplayer.h"
#include "mediaplayer_p.h"
#include "utils.h"
#include "macros.h"

namespace BluezQt
{

GattCharacteristicRemotePrivate::GattCharacteristicRemotePrivate(const QString &path, const QVariantMap &properties, const GattServiceRemotePtr &service)
    : QObject()
    , m_dbusProperties(nullptr)
    , m_writeAcquired(false)
    , m_notifyAcquired(false)
    , m_notifying(false)
    , m_handle()
    , m_MTU()
    , m_service(service)
{
    m_bluezGattCharacteristic = new BluezGattCharacteristic(Strings::orgBluez(), path, DBusConnection::orgBluez(), this);

    init(properties);
}

void GattCharacteristicRemotePrivate::init(const QVariantMap &properties)
{
    m_dbusProperties = new DBusProperties(Strings::orgBluez(), m_bluezGattCharacteristic->path(),
                                          DBusConnection::orgBluez(), this);

    // Init properties
    m_uuid = properties.value(QStringLiteral("UUID")).toString();
    m_value = properties.value(QStringLiteral("Value")).toByteArray();
    m_writeAcquired = properties.value(QStringLiteral("WriteAcquired")).toBool();
    m_notifyAcquired = properties.value(QStringLiteral("NotifyAcquired")).toBool();
    m_notifying = properties.value(QStringLiteral("Notifying")).toBool();
    m_flags = properties.value(QStringLiteral("Flags")).toStringList();
    m_handle = properties.value(QStringLiteral("Handle")).value<quint16>();
    m_MTU = properties.value(QStringLiteral("MTU")).value<quint16>();
}

void GattCharacteristicRemotePrivate::interfacesAdded(const QString &path, const QVariantMapMap &interfaces)
{
    bool changed = false;
    QVariantMapMap::const_iterator it;

    for (it = interfaces.constBegin(); it != interfaces.constEnd(); ++it) {
        if (it.key() == Strings::orgBluezGattDescriptor1()) {
            addGattDescriptor(path,it.value());
            changed = true;
        }
    }

    if (changed) {
        Q_EMIT q.lock().data()->characteristicChanged(q.toStrongRef());
    }
}

void GattCharacteristicRemotePrivate::interfacesRemoved(const QString &path, const QStringList &interfaces)
{
    Q_UNUSED(path)
    bool changed = false;

    for (auto& interface : interfaces) {
        if (interface == Strings::orgBluezGattDescriptor1()) {
            removeGattDescriptor(path);
            changed = true;
        }
    }

    if (changed) {        
        Q_EMIT q.lock().data()->characteristicChanged(q.toStrongRef());
    }
}

void GattCharacteristicRemotePrivate::addGattDescriptor(const QString &gattDescriptorPath, const QVariantMap &properties)
{
    // Check if we have the right path
    if (m_bluezGattCharacteristic->path() != properties.value(QStringLiteral("Characteristic")).value<QDBusObjectPath>().path()) {
        return;
    }

    GattCharacteristicRemotePtr characteristic = GattCharacteristicRemotePtr(this->q);

    if (!characteristic) {
        return;
    }

    GattDescriptorRemotePtr gattDescriptor = GattDescriptorRemotePtr(new GattDescriptorRemote(gattDescriptorPath, properties, characteristic));
    gattDescriptor->d->q = gattDescriptor.toWeakRef();
    m_descriptors.append(gattDescriptor);

    Q_EMIT characteristic->gattDescriptorAdded(gattDescriptor);
    Q_EMIT characteristic->descriptorsChanged(m_descriptors);

    // Connections    
    connect(gattDescriptor.data(),&GattDescriptorRemote::descriptorChanged,q.lock().data(),&GattCharacteristicRemote::gattDescriptorChanged);
}

void GattCharacteristicRemotePrivate::removeGattDescriptor(const QString &gattDescriptorPath)
{
    GattCharacteristicRemotePtr characteristic = GattCharacteristicRemotePtr(this->q);

    if (!characteristic) {
        return;
    }

    GattDescriptorRemotePtr gattDescriptor = nullptr;
    for (int i=0; i < characteristic->descriptors().size(); ++i) {
        if (characteristic->descriptors().at(i)->ubi() == gattDescriptorPath) {
            gattDescriptor = characteristic->descriptors().at(i);
        }
    }

    if (gattDescriptor == nullptr) {
        return;
    }

    m_descriptors.removeOne(gattDescriptor);

    Q_EMIT characteristic->gattDescriptorRemoved(gattDescriptor);
    Q_EMIT characteristic->descriptorsChanged(m_descriptors);

    // Connections    
    disconnect(gattDescriptor.data(),&GattDescriptorRemote::descriptorChanged,q.lock().data(),&GattCharacteristicRemote::gattDescriptorChanged);
}

QDBusPendingReply<> GattCharacteristicRemotePrivate::setDBusProperty(const QString &name, const QVariant &value)
{
    return m_dbusProperties->Set(Strings::orgBluezGattCharacteristic1(), name, QDBusVariant(value));
}

void GattCharacteristicRemotePrivate::propertiesChanged(const QString &path, const QString &interface, const QVariantMap &changed, const QStringList &invalidated)
{
    Q_UNUSED(path)

    if (interface == Strings::orgBluezGattDescriptor1()) {
        for (GattDescriptorRemotePtr descriptor : m_descriptors) {
            if (path.startsWith(descriptor->ubi())) {
                descriptor->d->propertiesChanged(path, interface, changed, invalidated);
                return;
            }
        }
    } else if (interface != Strings::orgBluezGattCharacteristic1()) {
        return;
    }

    QVariantMap::const_iterator i;
    for (i = changed.constBegin(); i != changed.constEnd(); ++i) {
        const QVariant &value = i.value();
        const QString &property = i.key();

        if (property == QLatin1String("UUID")) {
            PROPERTY_CHANGED(m_uuid, toString, uuidChanged);
        } else if (property == QLatin1String("Value")) {
            PROPERTY_CHANGED(m_value, toByteArray, valueChanged);
        } else if (property == QLatin1String("WriteAcquired")) {
            PROPERTY_CHANGED(m_writeAcquired, toBool, writeAcquiredChanged);
        } else if (property == QLatin1String("NotifyAcquired")) {
            PROPERTY_CHANGED(m_writeAcquired, toBool, writeAcquiredChanged);
        } else if (property == QLatin1String("Notifying")) {
            PROPERTY_CHANGED(m_notifying, toBool, notifyingChanged);
        } else if (property == QLatin1String("Flags")) {
            PROPERTY_CHANGED2(m_flags, value.toStringList(), flagsChanged);
        } else if (property == QLatin1String("Handle")) {
            PROPERTY_CHANGED2(m_handle, value.value<quint16>(), handleChanged);
        } else if (property == QLatin1String("MTU")) {
            PROPERTY_CHANGED2(m_MTU, value.value<quint16>(), MTUChanged);
        }
    }

    for (auto& property : invalidated) {
        if (property == QLatin1String("UUID")) {
            PROPERTY_INVALIDATED(m_uuid, QString(), uuidChanged);
        } else if (property == QLatin1String("Value")) {
            PROPERTY_INVALIDATED(m_value, QByteArray(), valueChanged);
        } else if (property == QLatin1String("WriteAcquired")) {
            PROPERTY_INVALIDATED(m_writeAcquired, false, writeAcquiredChanged);
        } else if (property == QLatin1String("NotifyAcquired")) {
            PROPERTY_INVALIDATED(m_writeAcquired, false, writeAcquiredChanged);
        } else if (property == QLatin1String("Notifying")) {
            PROPERTY_INVALIDATED(m_notifying, false, notifyingChanged);
        } else if (property == QLatin1String("Flags")) {
            PROPERTY_INVALIDATED(m_flags, QStringList(), flagsChanged);
        } else if (property == QLatin1String("Handle")) {
            PROPERTY_INVALIDATED(m_handle, quint16(), handleChanged);
        } else if (property == QLatin1String("MTU")) {
            PROPERTY_INVALIDATED(m_MTU, quint16(), MTUChanged);
        }
    }

    q.lock().data()->characteristicChanged(q.toStrongRef());
}

} // namespace BluezQt

#include "moc_gattcharacteristicremote_p.cpp"
