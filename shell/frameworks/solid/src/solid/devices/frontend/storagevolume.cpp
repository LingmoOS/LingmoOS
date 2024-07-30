/*
    SPDX-FileCopyrightText: 2006-2007 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "storagevolume.h"
#include "storagevolume_p.h"

#include "soliddefs_p.h"
#include <solid/device.h>
#include <solid/devices/ifaces/storagevolume.h>

Solid::StorageVolume::StorageVolume(QObject *backendObject)
    : DeviceInterface(*new StorageVolumePrivate(), backendObject)
{
}

Solid::StorageVolume::StorageVolume(StorageVolumePrivate &dd, QObject *backendObject)
    : DeviceInterface(dd, backendObject)
{
}

Solid::StorageVolume::~StorageVolume()
{
}

bool Solid::StorageVolume::isIgnored() const
{
    Q_D(const StorageVolume);
    return_SOLID_CALL(Ifaces::StorageVolume *, d->backendObject(), true, isIgnored());
}

Solid::StorageVolume::UsageType Solid::StorageVolume::usage() const
{
    Q_D(const StorageVolume);
    return_SOLID_CALL(Ifaces::StorageVolume *, d->backendObject(), Unused, usage());
}

QString Solid::StorageVolume::fsType() const
{
    Q_D(const StorageVolume);
    return_SOLID_CALL(Ifaces::StorageVolume *, d->backendObject(), QString(), fsType());
}

QString Solid::StorageVolume::label() const
{
    Q_D(const StorageVolume);
    return_SOLID_CALL(Ifaces::StorageVolume *, d->backendObject(), QString(), label());
}

QString Solid::StorageVolume::uuid() const
{
    Q_D(const StorageVolume);
    return_SOLID_CALL(Ifaces::StorageVolume *, d->backendObject(), QString(), uuid().toLower());
}

qulonglong Solid::StorageVolume::size() const
{
    Q_D(const StorageVolume);
    return_SOLID_CALL(Ifaces::StorageVolume *, d->backendObject(), 0, size());
}

Solid::Device Solid::StorageVolume::encryptedContainer() const
{
    Q_D(const StorageVolume);

    Ifaces::StorageVolume *iface = qobject_cast<Ifaces::StorageVolume *>(d->backendObject());

    if (iface != nullptr) {
        return Device(iface->encryptedContainerUdi());
    } else {
        return Device();
    }
}

#include "moc_storagevolume.cpp"
