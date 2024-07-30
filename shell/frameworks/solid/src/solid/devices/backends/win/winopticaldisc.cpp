/*
    SPDX-FileCopyrightText: 2013 Patrick von Reth <vonreth@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "winopticaldisc.h"
#include "winopticaldrive.h"

#include <ntddcdrm.h>
#include <ntddmmc.h>

using namespace Solid::Backends::Win;

WinOpticalDisc::WinOpticalDisc(WinDevice *device)
    : WinStorageVolume(device)
    , m_discType(Solid::OpticalDisc::UnknownDiscType)
    , m_isRewritable(false)
{
    // TODO: blueray etc
    QMap<ulong, MediaProfiles> profiles = MediaProfiles::profiles(WinBlock::driveLetterFromUdi(m_device->udi()));

    if (profiles[ProfileCdRecordable].active) {
        m_discType = Solid::OpticalDisc::CdRecordable;
    } else if (profiles[ProfileCdRewritable].active) {
        m_discType = Solid::OpticalDisc::CdRewritable;
        m_isRewritable = true;
    } else if (profiles[ProfileCdrom].active) {
        m_discType = Solid::OpticalDisc::CdRom;
    } else if (profiles[ProfileDvdRecordable].active) {
        m_discType = Solid::OpticalDisc::DvdRecordable;
    } else if (profiles[ProfileDvdRewritable].active) {
        m_discType = Solid::OpticalDisc::DvdRewritable;
        m_isRewritable = true;
    } else if (profiles[ProfileDvdRom].active) {
        m_discType = Solid::OpticalDisc::DvdRom;
    } else {
        m_discType = Solid::OpticalDisc::UnknownDiscType;
    }
}

WinOpticalDisc::~WinOpticalDisc()
{
}

Solid::OpticalDisc::ContentTypes WinOpticalDisc::availableContent() const
{
    return Solid::OpticalDisc::NoContent;
}

Solid::OpticalDisc::DiscType WinOpticalDisc::discType() const
{
    return m_discType;
}

bool WinOpticalDisc::isAppendable() const
{
    return false;
}

bool WinOpticalDisc::isBlank() const
{
    wchar_t dLetter[MAX_PATH];
    int dLetterSize = WinBlock::driveLetterFromUdi(m_device->udi()).toWCharArray(dLetter);
    dLetter[dLetterSize] = (wchar_t)'\\';
    dLetter[dLetterSize + 1] = 0;

    ULARGE_INTEGER sizeTotal;
    ULARGE_INTEGER sizeFree;
    if (GetDiskFreeSpaceEx(dLetter, &sizeFree, &sizeTotal, NULL)) {
        return sizeFree.QuadPart > 0 && sizeTotal.QuadPart == 0;
    }
    // FIXME: the call will fail on a blank cd, and inf there is no cd, but if we got a disc type we could guess that it is a blank cd
    return m_discType != Solid::OpticalDisc::UnknownDiscType;
}

bool WinOpticalDisc::isRewritable() const
{
    return m_isRewritable;
}

qulonglong WinOpticalDisc::capacity() const
{
    return size();
}

#include "moc_winopticaldisc.cpp"
