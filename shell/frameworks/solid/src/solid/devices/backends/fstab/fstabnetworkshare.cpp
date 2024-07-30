/*
    SPDX-FileCopyrightText: 2011 Mario Bensi <mbensi@ipsquad.net>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "fstabnetworkshare.h"
#include "fstabhandling.h"
#include <solid/devices/backends/fstab/fstabdevice.h>

using namespace Solid::Backends::Fstab;

FstabNetworkShare::FstabNetworkShare(Solid::Backends::Fstab::FstabDevice *device)
    : QObject(device)
    , m_fstabDevice(device)
{
    QString url;
    if (m_fstabDevice->device().startsWith("//")) {
        QString fsType = FstabHandling::fstype(m_fstabDevice->device());
        if (fsType == "cifs") {
            m_type = Solid::NetworkShare::Cifs;
        } else if (fsType == "smb3") {
            m_type = Solid::NetworkShare::Smb3;
        }
        url = "smb:";
        url += m_fstabDevice->device();
    } else if (m_fstabDevice->device().contains(":/")) {
        m_type = Solid::NetworkShare::Nfs;
        url = "nfs://";
        url += m_fstabDevice->vendor() + "/";
        url += m_fstabDevice->product();
    } else {
        m_type = Solid::NetworkShare::Unknown;
    }
    m_url = QUrl(url);
}

FstabNetworkShare::~FstabNetworkShare()
{
}

Solid::NetworkShare::ShareType FstabNetworkShare::type() const
{
    return m_type;
}

QUrl FstabNetworkShare::url() const
{
    return m_url;
}

const Solid::Backends::Fstab::FstabDevice *FstabNetworkShare::fstabDevice() const
{
    return m_fstabDevice;
}

#include "moc_fstabnetworkshare.cpp"
