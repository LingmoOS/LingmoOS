/*
    SPDX-FileCopyrightText: 2020 MBition GmbH
    SPDX-FileContributor: Kai Uwe Broulik <kai_uwe.broulik@mbition.io>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "imobileportablemediaplayer.h"

#include "imobile.h"
#include "imobiledevice.h"

using namespace Solid::Backends::IMobile;

PortableMediaPlayer::PortableMediaPlayer(IMobileDevice *device)
    : DeviceInterface(device)
{
}

PortableMediaPlayer::~PortableMediaPlayer()
{
}

QStringList PortableMediaPlayer::supportedProtocols() const
{
    return {QStringLiteral("afc")};
}

QStringList PortableMediaPlayer::supportedDrivers(QString protocol) const
{
    Q_UNUSED(protocol)
    return {// libimobiledevice goes via usbmuxd
            QStringLiteral("usbmux")};
}

QVariant PortableMediaPlayer::driverHandle(const QString &driver) const
{
    if (driver == QLatin1String("usbmux")) {
        // as per docs "usbmux" should return the device UDID
        // which we use as part of our UDI
        return m_device->udi().mid(udiPrefix().length() + 1);
    }

    return {};
}

#include "moc_imobileportablemediaplayer.cpp"
