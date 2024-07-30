/*
    SPDX-FileCopyrightText: 2006 Davide Bettio <davide.bettio@kdemail.net>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "fakestorage.h"
#include <QVariant>

using namespace Solid::Backends::Fake;

FakeStorage::FakeStorage(FakeDevice *device)
    : FakeBlock(device)
{
}

FakeStorage::~FakeStorage()
{
}

Solid::StorageDrive::Bus FakeStorage::bus() const
{
    QString bus = fakeDevice()->property("bus").toString();

    if (bus == "ide") {
        return Solid::StorageDrive::Ide;
    } else if (bus == "usb") {
        return Solid::StorageDrive::Usb;
    } else if (bus == "ieee1394") {
        return Solid::StorageDrive::Ieee1394;
    } else if (bus == "scsi") {
        return Solid::StorageDrive::Scsi;
    } else if (bus == "sata") {
        return Solid::StorageDrive::Sata;
    } else {
        return Solid::StorageDrive::Platform;
    }
}

Solid::StorageDrive::DriveType FakeStorage::driveType() const
{
    QString type = fakeDevice()->property("major").toString();

    if (type == "disk") {
        return Solid::StorageDrive::HardDisk;
    } else if (type == "cdrom") {
        return Solid::StorageDrive::CdromDrive;
    } else if (type == "floppy") {
        return Solid::StorageDrive::Floppy;
    } else if (type == "tape") {
        return Solid::StorageDrive::Tape;
    } else if (type == "compact_flash") {
        return Solid::StorageDrive::CompactFlash;
    } else if (type == "memory_stick") {
        return Solid::StorageDrive::MemoryStick;
    } else if (type == "smart_media") {
        return Solid::StorageDrive::SmartMedia;
    } else if (type == "sd_mmc") {
        return Solid::StorageDrive::SdMmc;
    } else {
        return Solid::StorageDrive::HardDisk;
    }
}

bool FakeStorage::isRemovable() const
{
    return fakeDevice()->property("isRemovable").toBool();
}

bool FakeStorage::isHotpluggable() const
{
    return fakeDevice()->property("isHotpluggable").toBool();
}

qulonglong FakeStorage::size() const
{
    return fakeDevice()->property("size").toULongLong();
}

QDateTime FakeStorage::timeDetected() const
{
    return fakeDevice()->property("timeDetected").toDateTime();
}

QDateTime FakeStorage::timeMediaDetected() const
{
    return fakeDevice()->property("timeMediaDetected").toDateTime();
}

#include "moc_fakestorage.cpp"
