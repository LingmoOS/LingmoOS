/*
    SPDX-FileCopyrightText: 2017 René J.V. Bertin <rjvbertin@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "iokitstorage.h"

#include <CoreFoundation/CoreFoundation.h>
#include <DiskArbitration/DiskArbitration.h>

using namespace Solid::Backends::IOKit;

IOKitStorage::IOKitStorage(IOKitDevice *device)
    : Block(device)
    , daDict(new DADictionary(device))
{
}

IOKitStorage::IOKitStorage(const IOKitDevice *device)
    : Block(device)
    , daDict(new DADictionary(device))
{
}

IOKitStorage::~IOKitStorage()
{
    delete daDict;
}

Solid::StorageDrive::Bus IOKitStorage::bus() const
{
    const QString udi = m_device->udi();
    // TODO: figure out how to return something useful here.
    if (udi.contains(QStringLiteral("/SATA@"))) {
        return Solid::StorageDrive::Sata;
    }
    if (udi.contains(QStringLiteral("/SDXC@"))) {
        // TODO: return something finer grained; the built-in card reader
        // is NOT connected via USB on Macs, for instance (but there's no PCI option)
        return Solid::StorageDrive::Usb;
    }
    if (udi.contains(QStringLiteral("/IOUSBInterface@"))) {
        return Solid::StorageDrive::Usb;
    }
    if (daDict->stringForKey(kDADiskDescriptionDeviceProtocolKey) == QStringLiteral("USB")) {
        return Solid::StorageDrive::Usb;
    }
    return Solid::StorageDrive::Platform;
}

Solid::StorageDrive::DriveType IOKitStorage::driveType() const
{
    const QString udi = m_device->udi();
    const QString type = m_device->property(QLatin1String("className")).toString();

    if (type == QStringLiteral("IOCDMedia") //
        || type == QStringLiteral("IOBDMedia") //
        || type == QStringLiteral("IODVDMedia")) {
        return Solid::StorageDrive::CdromDrive;
    }
    if (udi.contains(QStringLiteral("/SDXC@"))) {
        return Solid::StorageDrive::SdMmc;
    }
    if (daDict->stringForKey(kDADiskDescriptionDeviceModelKey) == QStringLiteral("Compact Flash")) {
        return Solid::StorageDrive::CompactFlash;
    }
    return Solid::StorageDrive::HardDisk;
}

bool IOKitStorage::isRemovable() const
{
    bool isInternal = false;
    daDict->boolForKey(kDADiskDescriptionDeviceInternalKey, isInternal);
    return !isInternal || m_device->property(QLatin1String("Removable")).toBool();
}

bool IOKitStorage::isHotpluggable() const
{
    const Solid::StorageDrive::DriveType type = driveType();
    return bus() == Solid::StorageDrive::Usb //
        || type == Solid::StorageDrive::CdromDrive //
        || type == Solid::StorageDrive::SdMmc;
}

qulonglong IOKitStorage::size() const
{
    return m_device->property(QLatin1String("Size")).toULongLong();
}

QString IOKitStorage::vendor() const
{
    return daDict->stringForKey(kDADiskDescriptionDeviceVendorKey);
}

QString IOKitStorage::product() const
{
    return daDict->stringForKey(kDADiskDescriptionDeviceModelKey);
}

QString IOKitStorage::description() const
{
    return daDict->stringForKey(kDADiskDescriptionMediaNameKey);
}

#include "moc_iokitstorage.cpp"
