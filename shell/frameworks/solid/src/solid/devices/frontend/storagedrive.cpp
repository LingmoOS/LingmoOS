/*
    SPDX-FileCopyrightText: 2006-2007 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "storagedrive.h"
#include "storagedrive_p.h"

#include "device.h"
#include "device_p.h"
#include "predicate.h"
#include "soliddefs_p.h"
#include "storageaccess.h"
#include <solid/devices/ifaces/storagedrive.h>

Solid::StorageDrive::StorageDrive(QObject *backendObject)
    : DeviceInterface(*new StorageDrivePrivate(), backendObject)
{
}

Solid::StorageDrive::StorageDrive(StorageDrivePrivate &dd, QObject *backendObject)
    : DeviceInterface(dd, backendObject)
{
}

Solid::StorageDrive::~StorageDrive()
{
}

Solid::StorageDrive::Bus Solid::StorageDrive::bus() const
{
    Q_D(const StorageDrive);
    return_SOLID_CALL(Ifaces::StorageDrive *, d->backendObject(), Platform, bus());
}

Solid::StorageDrive::DriveType Solid::StorageDrive::driveType() const
{
    Q_D(const StorageDrive);
    return_SOLID_CALL(Ifaces::StorageDrive *, d->backendObject(), HardDisk, driveType());
}

bool Solid::StorageDrive::isRemovable() const
{
    Q_D(const StorageDrive);
    return_SOLID_CALL(Ifaces::StorageDrive *, d->backendObject(), false, isRemovable());
}

bool Solid::StorageDrive::isHotpluggable() const
{
    Q_D(const StorageDrive);
    return_SOLID_CALL(Ifaces::StorageDrive *, d->backendObject(), false, isHotpluggable());
}

qulonglong Solid::StorageDrive::size() const
{
    Q_D(const StorageDrive);
    return_SOLID_CALL(Ifaces::StorageDrive *, d->backendObject(), false, size());
}

bool Solid::StorageDrive::isInUse() const
{
    Q_D(const StorageDrive);
    Predicate p(DeviceInterface::StorageAccess);
    const QList<Device> devices = Device::listFromQuery(p, d->devicePrivate()->udi());

    bool inUse = false;
    for (const Device &dev : devices) {
        if (dev.is<Solid::StorageAccess>()) {
            const Solid::StorageAccess *access = dev.as<Solid::StorageAccess>();
            inUse |= (access->isAccessible());
        }
    }
    return inUse;
}

QDateTime Solid::StorageDrive::timeDetected() const
{
    Q_D(const StorageDrive);
    return_SOLID_CALL(Ifaces::StorageDrive *, d->backendObject(), QDateTime(), timeDetected());
}

QDateTime Solid::StorageDrive::timeMediaDetected() const
{
    Q_D(const StorageDrive);
    return_SOLID_CALL(Ifaces::StorageDrive *, d->backendObject(), QDateTime(), timeMediaDetected());
}

#include "moc_storagedrive.cpp"
