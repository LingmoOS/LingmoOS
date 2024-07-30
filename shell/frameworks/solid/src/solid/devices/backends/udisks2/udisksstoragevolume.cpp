/*
    SPDX-FileCopyrightText: 2010 Michael Zanetti <mzanetti@kde.org>
    SPDX-FileCopyrightText: 2010-2012 Lukáš Tinkl <ltinkl@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "udisksstoragevolume.h"
#include "udisks2.h"

#include <QDir>

using namespace Solid::Backends::UDisks2;

StorageVolume::StorageVolume(Device *device)
    : Block(device)
{
}

StorageVolume::~StorageVolume()
{
}

QString StorageVolume::encryptedContainerUdi() const
{
    const QString path = m_device->prop("CryptoBackingDevice").value<QDBusObjectPath>().path();
    if (path.isEmpty() || path == "/") {
        return QString();
    } else {
        return path;
    }
}

qulonglong StorageVolume::size() const
{
    return m_device->prop("Size").toULongLong();
}

QString StorageVolume::uuid() const
{
    return m_device->prop("IdUUID").toString();
}

QString StorageVolume::label() const
{
    QString label = m_device->prop("HintName").toString();
    if (label.isEmpty()) {
        label = m_device->prop("IdLabel").toString();
    }
    if (label.isEmpty()) {
        label = m_device->prop("Name").toString();
    }
    return label;
}

QString StorageVolume::fsType() const
{
    return m_device->prop("IdType").toString();
}

Solid::StorageVolume::UsageType StorageVolume::usage() const
{
    const QString usage = m_device->prop("IdUsage").toString();

    if (m_device->hasInterface(UD2_DBUS_INTERFACE_FILESYSTEM)) {
        return Solid::StorageVolume::FileSystem;
    } else if (m_device->isPartitionTable()) {
        return Solid::StorageVolume::PartitionTable;
    } else if (usage == "raid") {
        return Solid::StorageVolume::Raid;
    } else if (m_device->isEncryptedContainer()) {
        return Solid::StorageVolume::Encrypted;
    } else if (usage == "unused" || usage.isEmpty()) {
        return Solid::StorageVolume::Unused;
    } else {
        return Solid::StorageVolume::Other;
    }
}

bool StorageVolume::isIgnored() const
{
    if (m_device->prop("HintIgnore").toBool()) {
        return true;
    }

    const QStringList mountOptions = m_device->prop("UserspaceMountOptions").toStringList();
    if (mountOptions.contains(QLatin1String("x-gdu.hide"))) {
        return true;
    }

    const Solid::StorageVolume::UsageType usg = usage();

    /* clang-format off */
    if (m_device->isSwap()
        || ((usg == Solid::StorageVolume::Unused
                || usg == Solid::StorageVolume::Other
                || usg == Solid::StorageVolume::PartitionTable)
            && !m_device->isOpticalDisc())) { /* clang-format on */
        return true;
    }

    const QString backingFile = m_device->prop("BackingFile").toString();
    return !backingFile.isEmpty() && !backingFile.startsWith(QDir::homePath());
}

#include "moc_udisksstoragevolume.cpp"
