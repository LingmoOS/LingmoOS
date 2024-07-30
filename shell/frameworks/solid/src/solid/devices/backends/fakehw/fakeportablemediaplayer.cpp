/*
    SPDX-FileCopyrightText: 2006 Davide Bettio <davide.bettio@kdemail.net>
    SPDX-FileCopyrightText: 2007 Jeff Mitchell <kde-dev@emailgoeshere.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "fakeportablemediaplayer.h"

#include <QStringList>

using namespace Solid::Backends::Fake;

FakePortableMediaPlayer::FakePortableMediaPlayer(FakeDevice *device)
    : FakeDeviceInterface(device)
{
}

FakePortableMediaPlayer::~FakePortableMediaPlayer()
{
}

QStringList FakePortableMediaPlayer::supportedProtocols() const
{
    return fakeDevice()->property("supportedProtocols").toString().simplified().split(',');
}

QStringList FakePortableMediaPlayer::supportedDrivers(QString protocol) const
{
    Q_UNUSED(protocol);
    return fakeDevice()->property("supportedDrivers").toString().simplified().split(',');
}

QVariant Solid::Backends::Fake::FakePortableMediaPlayer::driverHandle(const QString &driver) const
{
    Q_UNUSED(driver);
    return QVariant();
}

#include "moc_fakeportablemediaplayer.cpp"
