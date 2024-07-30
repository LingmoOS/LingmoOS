/*
    SPDX-FileCopyrightText: 2017 René J.V. Bertin <rjvbertin@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "iokitvolume.h"
#include "iokitgenericinterface.h"

#include <CoreFoundation/CoreFoundation.h>

using namespace Solid::Backends::IOKit;

IOKitVolume::IOKitVolume(IOKitDevice *device)
    : Block(device)
    , daDict(new DADictionary(device))
{
}

IOKitVolume::IOKitVolume(const IOKitDevice *device)
    : Block(device)
    , daDict(new DADictionary(device))
{
}

IOKitVolume::~IOKitVolume()
{
    delete daDict;
}

bool IOKitVolume::isIgnored() const
{
    // ignore storage volumes that aren't mounted
    bool isIgnored = m_device->property(QStringLiteral("Open")).toBool() == false;
    m_device->setProperty("isIgnored", isIgnored);
    m_device->setProperty("isMounted", !isIgnored);
    return isIgnored;
}

Solid::StorageVolume::UsageType IOKitVolume::usage() const
{
    const QString content = m_device->property(QStringLiteral("Content")).toString();
    if (content == QStringLiteral("CD_DA")) {
        // this is (probably) a CD track
        return Solid::StorageVolume::Other;
    }
    if (content.contains(QStringLiteral("partition_scheme"))) {
        return Solid::StorageVolume::PartitionTable;
    }
    return Solid::StorageVolume::FileSystem;
}

QString IOKitVolume::fsType() const
{
    return daDict->stringForKey(kDADiskDescriptionVolumeKindKey);
}

QString IOKitVolume::label() const
{
    return daDict->stringForKey(kDADiskDescriptionVolumeNameKey);
}

QString IOKitVolume::uuid() const
{
    return m_device->property(QStringLiteral("UUID")).toString();
}

qulonglong IOKitVolume::size() const
{
    return m_device->property(QStringLiteral("Size")).toULongLong();
}

QString IOKitVolume::encryptedContainerUdi() const
{
    return QString();
}

QString IOKitVolume::vendor() const
{
    return daDict->stringForKey(kDADiskDescriptionDeviceVendorKey);
}

QString IOKitVolume::product() const
{
    return daDict->stringForKey(kDADiskDescriptionDeviceModelKey);
}

QString IOKitVolume::description() const
{
    return daDict->stringForKey(kDADiskDescriptionMediaNameKey);
}

DADiskRef IOKitVolume::daRef() const
{
    return daDict->daRef;
}

#include "moc_iokitvolume.cpp"
