/*
    SPDX-FileCopyrightText: 2013 Patrick von Reth <vonreth@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "winstoragevolume.h"

#include "windevicemanager.h"
#include "winutils_p.h"

#include <qt_windows.h>

#include <iostream>

using namespace Solid::Backends::Win;

WinStorageVolume::WinStorageVolume(WinDevice *device)
    : WinBlock(device)
    , m_size(0)
{
    updateCache();
}

WinStorageVolume::~WinStorageVolume()
{
}

bool WinStorageVolume::isIgnored() const
{
    return WinBlock::driveLetterFromUdi(m_device->udi()).isNull();
}

Solid::StorageVolume::UsageType WinStorageVolume::usage() const
{
    return Solid::StorageVolume::FileSystem; // TODO:???
}

void WinStorageVolume::updateCache()
{
    wchar_t label[MAX_PATH];
    wchar_t fs[MAX_PATH];
    DWORD serial;
    DWORD flags;
    // TODO:get correct name
    wchar_t dLetter[MAX_PATH];
    int dLetterSize = WinBlock::driveLetterFromUdi(m_device->udi()).toWCharArray(dLetter);
    dLetter[dLetterSize] = (wchar_t)'\\';
    dLetter[dLetterSize + 1] = 0;

    // block error dialogs from GetDiskFreeSpaceEx & other WinAPI, see bug 371012
    WinErrorBlocker block;

    if (GetVolumeInformation(dLetter, label, MAX_PATH, &serial, NULL, &flags, fs, MAX_PATH)) {
        m_label = QString::fromWCharArray(label);
        m_fs = QString::fromWCharArray(fs);
        m_uuid = QString::number(serial, 16);
    }

    ULARGE_INTEGER size;
    if (GetDiskFreeSpaceEx(dLetter, NULL, &size, NULL)) {
        m_size = size.QuadPart;
    }
}

QString WinStorageVolume::fsType() const
{
    return m_fs;
}

QString WinStorageVolume::label() const
{
    return m_label;
}

QString WinStorageVolume::uuid() const
{
    return m_uuid;
}

qulonglong WinStorageVolume::size() const
{
    return m_size;
}

QString WinStorageVolume::encryptedContainerUdi() const
{
    return QString();
}

#include "moc_winstoragevolume.cpp"
