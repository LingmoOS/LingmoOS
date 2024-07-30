/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * SPDX-FileCopyrightText: 2014 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "adapter_p.h"
#include "adapter.h"
#include "gattmanager.h"
#include "leadvertisingmanager.h"
#include "leadvertisingmanager_p.h"
#include "macros.h"
#include "media.h"
#include "media_p.h"
#include "utils.h"

namespace BluezQt
{
AdapterPrivate::AdapterPrivate(const QString &path, const QVariantMap &properties)
    : QObject()
    , m_dbusProperties(nullptr)
    , m_adapterClass(0)
    , m_powered(0)
    , m_discoverable(false)
    , m_discoverableTimeout(0)
    , m_pairable(false)
    , m_pairableTimeout(0)
{
    m_bluezAdapter = new BluezAdapter(Strings::orgBluez(), path, DBusConnection::orgBluez(), this);

    init(properties);
}

void AdapterPrivate::init(const QVariantMap &properties)
{
    m_dbusProperties = new DBusProperties(Strings::orgBluez(), m_bluezAdapter->path(), DBusConnection::orgBluez(), this);

    // Init properties
    m_address = properties.value(QStringLiteral("Address")).toString();
    m_name = properties.value(QStringLiteral("Name")).toString();
    m_alias = properties.value(QStringLiteral("Alias")).toString();
    m_adapterClass = properties.value(QStringLiteral("Class")).toUInt();
    m_powered = properties.value(QStringLiteral("Powered")).toBool();
    m_discoverable = properties.value(QStringLiteral("Discoverable")).toBool();
    m_discoverableTimeout = properties.value(QStringLiteral("DiscoverableTimeout")).toUInt();
    m_pairable = properties.value(QStringLiteral("Pairable")).toBool();
    m_pairableTimeout = properties.value(QStringLiteral("PairableTimeout")).toUInt();
    m_discovering = properties.value(QStringLiteral("Discovering")).toBool();
    m_uuids = stringListToUpper(properties.value(QStringLiteral("UUIDs")).toStringList());
    m_modalias = properties.value(QStringLiteral("Modalias")).toString();
}

void AdapterPrivate::interfacesAdded(const QString &path, const QVariantMapMap &interfaces)
{
    bool changed = false;

    for (auto it = interfaces.cbegin(); it != interfaces.cend(); ++it) {
        if (it.key() == Strings::orgBluezMedia1()) {
            m_media = MediaPtr(new Media(path));
            Q_EMIT q.lock()->mediaChanged(m_media);
            changed = true;
        } else if (it.key() == Strings::orgBluezLEAdvertisingManager1()) {
            m_leAdvertisingManager = LEAdvertisingManagerPtr(new LEAdvertisingManager(path));
            Q_EMIT q.lock()->leAdvertisingManagerChanged(m_leAdvertisingManager);
            changed = true;
        } else if (it.key() == Strings::orgBluezGattManager1()) {
            m_gattManager = GattManagerPtr(new GattManager(path));
            Q_EMIT q.lock()->gattManagerChanged(m_gattManager);
            changed = true;
        }
    }

    if (changed) {
        Q_EMIT q.lock()->adapterChanged(q.toStrongRef());
    }
}

void AdapterPrivate::interfacesRemoved(const QString &path, const QStringList &interfaces)
{
    bool changed = false;

    for (const QString &interface : interfaces) {
        if (interface == Strings::orgBluezMedia1() && m_media && m_media->d->m_path == path) {
            m_media.clear();
            Q_EMIT q.lock()->mediaChanged(m_media);
            changed = true;
        } else if (interface == Strings::orgBluezLEAdvertisingManager1() && m_leAdvertisingManager && m_leAdvertisingManager->d->m_path == path) {
            m_leAdvertisingManager.clear();
            Q_EMIT q.lock()->leAdvertisingManagerChanged(m_leAdvertisingManager);
            changed = true;
        }
    }

    if (changed) {
        Q_EMIT q.lock()->adapterChanged(q.toStrongRef());
    }
}

void AdapterPrivate::addDevice(const DevicePtr &device)
{
    m_devices.append(device);
    Q_EMIT q.lock()->deviceAdded(device);

    connect(device.data(), &Device::deviceChanged, q.lock().data(), &Adapter::deviceChanged);
}

void AdapterPrivate::removeDevice(const DevicePtr &device)
{
    m_devices.removeOne(device);
    Q_EMIT device->deviceRemoved(device);
    Q_EMIT q.lock()->deviceRemoved(device);

    disconnect(device.data(), &Device::deviceChanged, q.lock().data(), &Adapter::deviceChanged);
}

QDBusPendingReply<> AdapterPrivate::setDBusProperty(const QString &name, const QVariant &value)
{
    return m_dbusProperties->Set(Strings::orgBluezAdapter1(), name, QDBusVariant(value));
}

void AdapterPrivate::propertiesChanged(const QString &interface, const QVariantMap &changed, const QStringList &invalidated)
{
    if (interface != Strings::orgBluezAdapter1()) {
        return;
    }

    QVariantMap::const_iterator i;
    for (i = changed.constBegin(); i != changed.constEnd(); ++i) {
        const QVariant &value = i.value();
        const QString &property = i.key();

        if (property == QLatin1String("Name")) {
            PROPERTY_CHANGED(m_name, toString, systemNameChanged);
        } else if (property == QLatin1String("Alias")) {
            PROPERTY_CHANGED(m_alias, toString, nameChanged);
        } else if (property == QLatin1String("Class")) {
            PROPERTY_CHANGED(m_adapterClass, toUInt, adapterClassChanged);
        } else if (property == QLatin1String("Powered")) {
            PROPERTY_CHANGED(m_powered, toBool, poweredChanged);
        } else if (property == QLatin1String("Discoverable")) {
            PROPERTY_CHANGED(m_discoverable, toBool, discoverableChanged);
        } else if (property == QLatin1String("DiscoverableTimeout")) {
            PROPERTY_CHANGED(m_discoverableTimeout, toUInt, discoverableTimeoutChanged);
        } else if (property == QLatin1String("Pairable")) {
            PROPERTY_CHANGED(m_pairable, toBool, pairableChanged);
        } else if (property == QLatin1String("PairableTimeout")) {
            PROPERTY_CHANGED(m_pairableTimeout, toUInt, pairableTimeoutChanged);
        } else if (property == QLatin1String("Discovering")) {
            PROPERTY_CHANGED(m_discovering, toBool, discoveringChanged);
        } else if (property == QLatin1String("Modalias")) {
            PROPERTY_CHANGED(m_modalias, toString, modaliasChanged);
        } else if (property == QLatin1String("UUIDs")) {
            PROPERTY_CHANGED2(m_uuids, stringListToUpper(value.toStringList()), uuidsChanged);
        }
    }

    for (const QString &property : invalidated) {
        if (property == QLatin1String("Modalias")) {
            PROPERTY_INVALIDATED(m_modalias, QString(), modaliasChanged);
        }
    }

    Q_EMIT q.lock()->adapterChanged(q.toStrongRef());
}

} // namespace BluezQt

#include "moc_adapter_p.cpp"
