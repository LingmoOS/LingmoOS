/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2011 Sebastian Trueg <trueg@kde.org>
    SPDX-FileCopyrightText: 2014 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "storagedevices.h"
#include "baloodebug.h"

#include <Solid/DeviceNotifier>
#include <Solid/DeviceInterface>
#include <Solid/Block>
#include <Solid/StorageDrive>
#include <Solid/StorageVolume>
#include <Solid/StorageAccess>
#include <Solid/NetworkShare>
#include <Solid/OpticalDisc>
#include <Solid/Predicate>

using namespace Baloo;

StorageDevices::StorageDevices(QObject* parent)
    : QObject(parent)
{
    connect(Solid::DeviceNotifier::instance(), &Solid::DeviceNotifier::deviceAdded,
            this, &StorageDevices::slotSolidDeviceAdded);
    connect(Solid::DeviceNotifier::instance(), &Solid::DeviceNotifier::deviceRemoved,
            this, &StorageDevices::slotSolidDeviceRemoved);

    initCacheEntries();
}

StorageDevices::~StorageDevices()
{
}

void StorageDevices::initCacheEntries()
{
    const QList<Solid::Device> devices = Solid::Device::allDevices();
    for (const Solid::Device& dev : devices) {
        createCacheEntry(dev);
    }
}

QList<StorageDevices::Entry> StorageDevices::allMedia() const
{
    return m_metadataCache.values();
}

StorageDevices::Entry* StorageDevices::createCacheEntry(const Solid::Device& dev)
{
    if (dev.udi().isEmpty()) {
        return nullptr;
    }

    const Solid::StorageAccess* storage = dev.as<Solid::StorageAccess>();
    if (!storage) {
        return nullptr;
    }

    Entry entry(dev);
    auto it = m_metadataCache.insert(dev.udi(), entry);

    connect(storage, &Solid::StorageAccess::accessibilityChanged,
            this, &StorageDevices::slotAccessibilityChanged);
    //connect(storage, SIGNAL(teardownRequested(QString)),
    //        this, SLOT(slotTeardownRequested(QString)));
    return &it.value();
}

bool StorageDevices::isEmpty() const
{
    return m_metadataCache.isEmpty();
}

void StorageDevices::slotSolidDeviceAdded(const QString& udi)
{
    qCDebug(BALOO) << udi;
    Entry* e = createCacheEntry(Solid::Device(udi));
    if (e) {
        Q_EMIT deviceAdded(e);
    }
}

void StorageDevices::slotSolidDeviceRemoved(const QString& udi)
{
    QHash< QString, Entry >::iterator it = m_metadataCache.find(udi);
    if (it != m_metadataCache.end()) {
        qCDebug(BALOO) << "Found removable storage volume for Baloo undocking:" << udi;
        Q_EMIT deviceRemoved(&it.value());
        m_metadataCache.erase(it);
    }
}

void StorageDevices::slotAccessibilityChanged(bool accessible, const QString& udi)
{
    qCDebug(BALOO) << accessible << udi;
    Q_UNUSED(accessible);

    //
    // cache new mount path
    //
    Entry* entry = &m_metadataCache[udi];
    Q_ASSERT(entry != nullptr);
    Q_EMIT deviceAccessibilityChanged(entry);
}

StorageDevices::Entry::Entry()
{
}

StorageDevices::Entry::Entry(const Solid::Device& device)
    : m_device(device)
{
}

QString StorageDevices::Entry::mountPath() const
{
    if (const Solid::StorageAccess* sa = m_device.as<Solid::StorageAccess>()) {
        return sa->filePath();
    } else {
        return QString();
    }
}

bool StorageDevices::Entry::isMounted() const
{
    if (const Solid::StorageAccess* sa = m_device.as<Solid::StorageAccess>()) {
        return sa->isAccessible();
    } else {
        return false;
    }
}

bool StorageDevices::Entry::isUsable() const
{
    if (mountPath().isEmpty()) {
        return false;
    }

    bool usable = true;

    const Solid::Device& dev = m_device;
    if (dev.is<Solid::StorageVolume>() && dev.parent().is<Solid::StorageDrive>()) {
        auto parent = dev.parent().as<Solid::StorageDrive>();
        if (parent->isRemovable() || parent->isHotpluggable()) {
            usable = false;
        }

        const Solid::StorageVolume* volume = dev.as<Solid::StorageVolume>();
        if (volume->isIgnored() || volume->usage() != Solid::StorageVolume::FileSystem) {
            usable = false;
        }
    }

    if (dev.is<Solid::NetworkShare>()) {
        usable = false;
    } else if (dev.is<Solid::OpticalDisc>()) {
        usable = false;
    } else if (dev.is<Solid::StorageAccess>() && dev.parentUdi() == QStringLiteral("/org/kde/fstab")) {
        usable = false;
    }

    if (usable) {
        if (const Solid::StorageAccess* sa = dev.as<Solid::StorageAccess>()) {
            usable = sa->isAccessible();
        }
    }

    return usable;
}

#include "moc_storagedevices.cpp"
