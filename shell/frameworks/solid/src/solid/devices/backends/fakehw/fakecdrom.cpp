/*
    SPDX-FileCopyrightText: 2006 Davide Bettio <davide.bettio@kdemail.net>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "fakecdrom.h"

#include <QStringList>

using namespace Solid::Backends::Fake;

FakeCdrom::FakeCdrom(FakeDevice *device)
    : FakeStorage(device)
{
}

FakeCdrom::~FakeCdrom()
{
}

Solid::OpticalDrive::MediumTypes FakeCdrom::supportedMedia() const
{
    Solid::OpticalDrive::MediumTypes supported;

    const QMap<QString, Solid::OpticalDrive::MediumType> map = {
        {QStringLiteral("cdr"), Solid::OpticalDrive::Cdr},
        {QStringLiteral("cdrw"), Solid::OpticalDrive::Cdrw},
        {QStringLiteral("dvd"), Solid::OpticalDrive::Dvd},
        {QStringLiteral("dvdr"), Solid::OpticalDrive::Dvdr},
        {QStringLiteral("dvdrw"), Solid::OpticalDrive::Dvdrw},
        {QStringLiteral("dvdram"), Solid::OpticalDrive::Dvdram},
        {QStringLiteral("dvdplusr"), Solid::OpticalDrive::Dvdplusr},
        {QStringLiteral("dvdplusrw"), Solid::OpticalDrive::Dvdplusrw},
        {QStringLiteral("dvdplusrdl"), Solid::OpticalDrive::Dvdplusdl},
        {QStringLiteral("dvdplusrwdl"), Solid::OpticalDrive::Dvdplusdlrw},
        {QStringLiteral("bd"), Solid::OpticalDrive::Bd},
        {QStringLiteral("bdr"), Solid::OpticalDrive::Bdr},
        {QStringLiteral("bdre"), Solid::OpticalDrive::Bdre},
        {QStringLiteral("hddvd"), Solid::OpticalDrive::HdDvd},
        {QStringLiteral("hddvdr"), Solid::OpticalDrive::HdDvdr},
        {QStringLiteral("hddvdrw"), Solid::OpticalDrive::HdDvdrw},
    };

    const QStringList supported_medialist = fakeDevice()->property("supportedMedia").toString().simplified().split(',');

    for (const QString &media : supported_medialist) {
        supported |= map.value(media, Solid::OpticalDrive::UnknownMediumType);
    }

    return supported;
}

int FakeCdrom::readSpeed() const
{
    return fakeDevice()->property("readSpeed").toInt();
}

int FakeCdrom::writeSpeed() const
{
    return fakeDevice()->property("writeSpeed").toInt();
}

QList<int> FakeCdrom::writeSpeeds() const
{
    QList<int> speeds;
    const QStringList speed_strlist = fakeDevice()->property("writeSpeeds").toString().simplified().split(',');

    for (const QString &speed_str : speed_strlist) {
        speeds << speed_str.toInt();
    }

    return speeds;
}

bool FakeCdrom::eject()
{
    return false;
}

#include "moc_fakecdrom.cpp"
