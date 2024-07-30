/*
    SPDX-FileCopyrightText: 2013 Patrick von Reth <vonreth@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#include "winstoragedrive.h"

#include <qt_windows.h>
#include <winioctl.h>

#if defined(__MINGW32__) && !defined(IOCTL_STORAGE_QUERY_PROPERTY)
#include <winioctl_backport.h>
#endif

using namespace Solid::Backends::Win;

WinStorageDrive::WinStorageDrive(WinDevice *device)
    : WinBlock(device)
{
    updateCache();
}

WinStorageDrive::~WinStorageDrive()
{
}

Solid::StorageDrive::Bus WinStorageDrive::bus() const
{
    return m_bus;
}

Solid::StorageDrive::DriveType WinStorageDrive::driveType() const
{
    if (m_device->type() == Solid::DeviceInterface::OpticalDrive) {
        return Solid::StorageDrive::CdromDrive;
    }
    if (bus() == Solid::StorageDrive::Usb) {
        return Solid::StorageDrive::MemoryStick;
    }
    return Solid::StorageDrive::HardDisk;
}

bool WinStorageDrive::isRemovable() const
{
    return m_isRemovable;
}

bool WinStorageDrive::isHotpluggable() const
{
    return m_isHotplugges;
}

qulonglong WinStorageDrive::size() const
{
    return m_size;
}

void WinStorageDrive::updateCache()
{
    STORAGE_PROPERTY_QUERY storageProperty;
    storageProperty.PropertyId = StorageAdapterProperty;
    storageProperty.QueryType = PropertyStandardQuery;

    QString dev;
    if (m_device->type() == Solid::DeviceInterface::OpticalDrive) {
        dev = WinBlock::driveLetterFromUdi(m_device->udi());
    } else {
        dev = QLatin1String("PhysicalDrive") + QString::number(deviceMajor());
    }
    STORAGE_ADAPTER_DESCRIPTOR busInfo = WinDeviceManager::getDeviceInfo<STORAGE_ADAPTER_DESCRIPTOR>(dev, IOCTL_STORAGE_QUERY_PROPERTY, &storageProperty);

    switch (busInfo.BusType) {
    case BusTypeUsb:
        m_bus = Solid::StorageDrive::Usb;
        break;
    case BusType1394:
        m_bus = Solid::StorageDrive::Ieee1394;
        break;
    case BusTypeScsi:
        m_bus = Solid::StorageDrive::Scsi;
        break;
    case BusTypeAta:
    default:
        m_bus = Solid::StorageDrive::Ide;
    }

    DISK_GEOMETRY sizeInfo = WinDeviceManager::getDeviceInfo<DISK_GEOMETRY>(dev, IOCTL_DISK_GET_DRIVE_GEOMETRY);
    m_size = sizeInfo.Cylinders.QuadPart * sizeInfo.TracksPerCylinder * sizeInfo.SectorsPerTrack * sizeInfo.BytesPerSector;

    STORAGE_HOTPLUG_INFO plugInfo = WinDeviceManager::getDeviceInfo<STORAGE_HOTPLUG_INFO>(dev, IOCTL_STORAGE_GET_HOTPLUG_INFO);
    m_isHotplugges = plugInfo.DeviceHotplug;
    m_isRemovable = plugInfo.MediaRemovable;
}

#include "moc_winstoragedrive.cpp"
