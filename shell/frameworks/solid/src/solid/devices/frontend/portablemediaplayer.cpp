/*
    SPDX-FileCopyrightText: 2006 Davide Bettio <davide.bettio@kdemail.net>
    SPDX-FileCopyrightText: 2007 Kevin Ottens <ervin@kde.org>
    SPDX-FileCopyrightText: 2007 Jeff Mitchell <kde-dev@emailgoeshere.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "portablemediaplayer.h"
#include "portablemediaplayer_p.h"

#include "soliddefs_p.h"
#include <solid/devices/ifaces/portablemediaplayer.h>

Solid::PortableMediaPlayer::PortableMediaPlayer(QObject *backendObject)
    : DeviceInterface(*new PortableMediaPlayerPrivate(), backendObject)
{
}

Solid::PortableMediaPlayer::~PortableMediaPlayer()
{
}

QStringList Solid::PortableMediaPlayer::supportedProtocols() const
{
    Q_D(const PortableMediaPlayer);
    return_SOLID_CALL(Ifaces::PortableMediaPlayer *, d->backendObject(), QStringList(), supportedProtocols());
}

QStringList Solid::PortableMediaPlayer::supportedDrivers(QString protocol) const
{
    Q_D(const PortableMediaPlayer);
    return_SOLID_CALL(Ifaces::PortableMediaPlayer *, d->backendObject(), QStringList(), supportedDrivers(protocol));
}

QVariant Solid::PortableMediaPlayer::driverHandle(const QString &driver) const
{
    Q_D(const PortableMediaPlayer);
    return_SOLID_CALL(Ifaces::PortableMediaPlayer *, d->backendObject(), QVariant(), driverHandle(driver));
}

#include "moc_portablemediaplayer.cpp"
