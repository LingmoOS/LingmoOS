/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * SPDX-FileCopyrightText: 2021 Ivan Podkurkov <podkiva2@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "gattserviceremote_p.h"
#include "gattserviceremote.h"
#include "gattcharacteristicremote_p.h"
#include "gattcharacteristicremote.h"
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

GattServiceRemotePrivate::GattServiceRemotePrivate(const QString &path, const QVariantMap &properties, const DevicePtr &device)
    : QObject()
    , m_dbusProperties(nullptr)
    , m_primary(false)
    , m_device(device)
    , m_handle(0)
{
    m_bluezGattService = new BluezGattService(Strings::orgBluez(), path, DBusConnection::orgBluez(), this);

    init(properties);
}

void GattServiceRemotePrivate::init(const QVariantMap &properties)
{
    m_dbusProperties = new DBusProperties(Strings::orgBluez(), m_bluezGattService->path(),
                                          DBusConnection::orgBluez(), this);

    // Init properties    
    m_uuid = properties.value(QStringLiteral("UUID")).toString();
    m_primary = properties.value(QStringLiteral("Primary")).toBool();
    m_includes = properties.value(QStringLiteral("Includes")).value<QList<QDBusObjectPath>>();
    m_handle = static_cast<quint16>(properties.value(QStringLiteral("Handle")).toUInt());

}

void GattServiceRemotePrivate::interfacesAdded(const QString &path, const QVariantMapMap &interfaces)
{
    bool changed = false;
    QVariantMapMap::const_iterator it;

    for (it = interfaces.constBegin(); it != interfaces.constEnd(); ++it) {
        if (it.key() == Strings::orgBluezGattCharacteristic1()) {
            addGattCharacteristic(path,it.value());
            changed = true;
        }
    }

    for (auto& ch: m_characteristics) {
        if (path.startsWith(ch->ubi())) {
            ch->d->interfacesAdded(path, interfaces);
            changed = true;
        }
    }

    if (changed) {
        Q_EMIT q.lock().data()->serviceChanged(q.toStrongRef());
    }
}

void GattServiceRemotePrivate::interfacesRemoved(const QString &path, const QStringList &interfaces)
{
    Q_UNUSED(path)
    bool changed = false;

    for (auto& interface : interfaces) {
        if (interface == Strings::orgBluezGattCharacteristic1()) {
            removeGattCharacteristic(path);
            changed = true;
        }
    }

    for (auto& ch: m_characteristics) {
        if (path.startsWith(ch->ubi())) {
            ch->d->interfacesRemoved(path, interfaces);
            changed = true;
        }
    }

    if (changed) {
        Q_EMIT q.lock().data()->serviceChanged(q.toStrongRef());
    }
}

void GattServiceRemotePrivate::addGattCharacteristic(const QString &gattCharacteristicPath, const QVariantMap &properties)
{
    // Check if we have the right path
    if (m_bluezGattService->path() != properties.value(QStringLiteral("Service")).value<QDBusObjectPath>().path()) {
        return;
    }

    GattServiceRemotePtr service = GattServiceRemotePtr(this->q);

    if (!service) {
        return;
    }

    GattCharacteristicRemotePtr gattCharacteristic = GattCharacteristicRemotePtr(new GattCharacteristicRemote(gattCharacteristicPath, properties, service));
    gattCharacteristic->d->q = gattCharacteristic.toWeakRef();
    m_characteristics.append(gattCharacteristic);

    Q_EMIT service->gattCharacteristicAdded(gattCharacteristic);
    Q_EMIT service->characteristicsChanged(m_characteristics);

    // Connections
    connect(gattCharacteristic.data(),&GattCharacteristicRemote::characteristicChanged,q.lock().data(),&GattServiceRemote::gattCharacteristicChanged);
}

void GattServiceRemotePrivate::removeGattCharacteristic(const QString &gattCharacteristicPath)
{
    GattServiceRemotePtr service = GattServiceRemotePtr(this->q);

    if (!service) {
        return;
    }

    GattCharacteristicRemotePtr gattCharacteristic = nullptr;
    for (int i=0; i < service->characteristics().size(); ++i) {
        if (service->characteristics().at(i)->ubi() == gattCharacteristicPath) {
            gattCharacteristic = service->characteristics().at(i);
        }
    }

    if (gattCharacteristic == nullptr) {
        return;
    }

    m_characteristics.removeOne(gattCharacteristic);

    Q_EMIT service->gattCharacteristicRemoved(gattCharacteristic);
    Q_EMIT service->characteristicsChanged(m_characteristics);

    // Connections
    disconnect(gattCharacteristic.data(),&GattCharacteristicRemote::characteristicChanged,q.lock().data(),&GattServiceRemote::gattCharacteristicChanged);
}

QDBusPendingReply<> GattServiceRemotePrivate::setDBusProperty(const QString &name, const QVariant &value)
{
    return m_dbusProperties->Set(Strings::orgBluezGattService1(), name, QDBusVariant(value));
}

void GattServiceRemotePrivate::propertiesChanged(const QString &path, const QString &interface, const QVariantMap &changed, const QStringList &invalidated)
{
    if (interface == Strings::orgBluezGattCharacteristic1() || interface == Strings::orgBluezGattDescriptor1()) {
        for (GattCharacteristicRemotePtr characteristic : m_characteristics) {
            if (path.startsWith(characteristic->ubi())) {
                characteristic->d->propertiesChanged(path, interface, changed, invalidated);
                return;
            }
        }
    } else if (interface != Strings::orgBluezGattService1()) {
        return;
    }

    QVariantMap::const_iterator i;
    for (i = changed.constBegin(); i != changed.constEnd(); ++i) {
        const QVariant &value = i.value();
        const QString &property = i.key();

        if (property == QLatin1String("UUID")) {
            PROPERTY_CHANGED(m_uuid, toString, uuidChanged);
        } else if (property == QLatin1String("Primary")) {
            PROPERTY_CHANGED(m_primary, toBool, primaryChanged);
        } else if (property == QLatin1String("Includes")) {
            PROPERTY_CHANGED2(m_includes, value.value<QList<QDBusObjectPath>>(), includesChanged);
        } else if (property == QLatin1String("Handle")) {
            PROPERTY_CHANGED2(m_handle, value.value<quint16>(), handleChanged);
        }
    }

    for (auto& property : invalidated) {
        if (property == QLatin1String("UUID")) {
            PROPERTY_INVALIDATED(m_uuid, QString(), uuidChanged);
        } else if (property == QLatin1String("Primary")) {
            PROPERTY_INVALIDATED(m_primary, false, primaryChanged);
        } else if (property == QLatin1String("Includes")) {
            PROPERTY_INVALIDATED(m_includes, QList<QDBusObjectPath>(), includesChanged);
        } else if (property == QLatin1String("Handle")) {
            PROPERTY_INVALIDATED(m_handle, quint16(), handleChanged);
        }
    }

    q.lock().data()->serviceChanged(q.toStrongRef());
}

} // namespace BluezQt

#include "moc_gattserviceremote_p.cpp"
