/*
    SPDX-FileCopyrightText: 2013 Patrick von Reth <vonreth@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "winstorageaccess.h"
#include "winblock.h"
#include "winstoragedrive.h"

using namespace Solid::Backends::Win;

WinStorageAccess::WinStorageAccess(WinDevice *device)
    : WinInterface(device)
{
}

WinStorageAccess::~WinStorageAccess()
{
}

bool WinStorageAccess::isAccessible() const
{
    return true;
}

QString WinStorageAccess::filePath() const
{
    return WinBlock::driveLetterFromUdi(m_device->udi()).append("/");
}

bool WinStorageAccess::isIgnored() const
{
    return false;
}

bool WinStorageAccess::isEncrypted() const
{
    // TODO: implementation left for Win developer
    return false;
}

bool WinStorageAccess::setup()
{
    return true;
}

bool WinStorageAccess::teardown()
{
    // only allow eject if we are an usb stick
    // else we get "The request could not be performed because of an I/O device error. 1117"
    if (m_device->queryDeviceInterface(Solid::DeviceInterface::StorageVolume) && WinStorageDrive(m_device).driveType() == Solid::StorageDrive::MemoryStick) {
        WinDeviceManager::deviceAction(WinBlock::driveLetterFromUdi(m_device->udi()), IOCTL_STORAGE_EJECT_MEDIA);
    }
    return true;
}

#include "moc_winstorageaccess.cpp"
