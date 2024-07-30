/*
    SPDX-FileCopyrightText: 2006 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_IFACES_STORAGEDRIVE_H
#define SOLID_IFACES_STORAGEDRIVE_H

#include <QDateTime>

#include <solid/devices/ifaces/block.h>
#include <solid/storagedrive.h>

namespace Solid
{
namespace Ifaces
{
/**
 * This device interface is available on storage devices.
 *
 * A storage is anything that can contain a set of volumes (card reader,
 * hard disk, cdrom drive...). It's a particular kind of block device.
 */
class StorageDrive : virtual public Block
{
public:
    /**
     * Destroys a StorageDrive object.
     */
    ~StorageDrive() override;

    /**
     * Retrieves the type of physical interface this storage device is
     * connected to.
     *
     * @return the bus type
     * @see Solid::StorageDrive::Bus
     */
    virtual Solid::StorageDrive::Bus bus() const = 0;

    /**
     * Retrieves the type of this storage drive.
     *
     * @return the drive type
     * @see Solid::StorageDrive::DriveType
     */
    virtual Solid::StorageDrive::DriveType driveType() const = 0;

    /**
     * Indicates if the media contained by this drive can be removed.
     *
     * For example memory card can be removed from the drive by the user,
     * while partitions can't be removed from hard disks.
     *
     * @return true if media can be removed, false otherwise.
     */
    virtual bool isRemovable() const = 0;

    /**
     * Indicates if this storage device can be plugged or unplugged while
     * the computer is running.
     *
     * @return true if this storage supports hotplug, false otherwise
     */
    virtual bool isHotpluggable() const = 0;

    /**
     * Retrieves this drives size in bytes.
     *
     * @return the size of this drive
     */
    virtual qulonglong size() const = 0;

    /**
     * Retrieves the time the drive was detected
     */
    virtual QDateTime timeDetected() const;

    /**
     * Retrieves the time media in the drive was detected
     */
    virtual QDateTime timeMediaDetected() const;
};
}
}

Q_DECLARE_INTERFACE(Solid::Ifaces::StorageDrive, "org.kde.Solid.Ifaces.StorageDrive/0.1")

#endif // SOLID_IFACES_STORAGEDRIVE_H
