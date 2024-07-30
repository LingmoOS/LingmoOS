/*
    SPDX-FileCopyrightText: 2006 Davide Bettio <davide.bettio@kdemail.net>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "fakeopticaldisc.h"

#include <QStringList>
#include <QVariant>

using namespace Solid::Backends::Fake;

FakeOpticalDisc::FakeOpticalDisc(FakeDevice *device)
    : FakeVolume(device)
{
}

FakeOpticalDisc::~FakeOpticalDisc()
{
}

Solid::OpticalDisc::ContentTypes FakeOpticalDisc::availableContent() const
{
    Solid::OpticalDisc::ContentTypes content;

    const QMap<QString, Solid::OpticalDisc::ContentType> map = {
        {QStringLiteral("audio"), Solid::OpticalDisc::Audio},
        {QStringLiteral("data"), Solid::OpticalDisc::Data},
        {QStringLiteral("vcd"), Solid::OpticalDisc::VideoCd},
        {QStringLiteral("svcd"), Solid::OpticalDisc::SuperVideoCd},
        {QStringLiteral("videodvd"), Solid::OpticalDisc::VideoDvd},
    };

    const QStringList content_typelist = fakeDevice()->property("availableContent").toString().split(',');

    for (const QString &type : content_typelist) {
        content |= map.value(type, Solid::OpticalDisc::NoContent);
    }

    return content;
}

Solid::OpticalDisc::DiscType FakeOpticalDisc::discType() const
{
    QString type = fakeDevice()->property("discType").toString();

    if (type == "cd_rom") {
        return Solid::OpticalDisc::CdRom;
    } else if (type == "cd_r") {
        return Solid::OpticalDisc::CdRecordable;
    } else if (type == "cd_rw") {
        return Solid::OpticalDisc::CdRewritable;
    } else if (type == "dvd_rom") {
        return Solid::OpticalDisc::DvdRom;
    } else if (type == "dvd_ram") {
        return Solid::OpticalDisc::DvdRam;
    } else if (type == "dvd_r") {
        return Solid::OpticalDisc::DvdRecordable;
    } else if (type == "dvd_rw") {
        return Solid::OpticalDisc::DvdRewritable;
    } else if (type == "dvd_plus_r") {
        return Solid::OpticalDisc::DvdPlusRecordable;
    } else if (type == "dvd_plus_rw") {
        return Solid::OpticalDisc::DvdPlusRewritable;
    } else if (type == "dvd_plus_r_dl") {
        return Solid::OpticalDisc::DvdPlusRecordableDuallayer;
    } else if (type == "dvd_plus_rw_dl") {
        return Solid::OpticalDisc::DvdPlusRewritableDuallayer;
    } else if (type == "bd_rom") {
        return Solid::OpticalDisc::BluRayRom;
    } else if (type == "bd_r") {
        return Solid::OpticalDisc::BluRayRecordable;
    } else if (type == "bd_re") {
        return Solid::OpticalDisc::BluRayRewritable;
    } else if (type == "hddvd_rom") {
        return Solid::OpticalDisc::HdDvdRom;
    } else if (type == "hddvd_r") {
        return Solid::OpticalDisc::HdDvdRecordable;
    } else if (type == "hddvd_rw") {
        return Solid::OpticalDisc::HdDvdRewritable;
    } else {
        return Solid::OpticalDisc::UnknownDiscType;
    }
}

bool FakeOpticalDisc::isAppendable() const
{
    return fakeDevice()->property("isAppendable").toBool();
}

bool FakeOpticalDisc::isBlank() const
{
    return fakeDevice()->property("isBlank").toBool();
}

bool FakeOpticalDisc::isRewritable() const
{
    return fakeDevice()->property("isRewritable").toBool();
}

qulonglong FakeOpticalDisc::capacity() const
{
    return fakeDevice()->property("capacity").toULongLong();
}

#include "moc_fakeopticaldisc.cpp"
