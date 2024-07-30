/*
    SPDX-FileCopyrightText: 2006 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_IFACES_STORAGEVOLUME_H
#define SOLID_IFACES_STORAGEVOLUME_H

#include <solid/devices/ifaces/block.h>
#include <solid/storagevolume.h>

namespace Solid
{
namespace Ifaces
{
/**
 * This device interface is available on volume devices.
 *
 * A volume is anything that can contain data (partition, optical disc,
 * memory card). It's a particular kind of block device.
 */
class StorageVolume : virtual public Block
{
public:
    /**
     * Destroys a StorageVolume object.
     */
    ~StorageVolume() override;

    /**
     * Indicates if this volume should be ignored by applications.
     *
     * If it should be ignored, it generally means that it should be
     * invisible to the user. It's useful for firmware partitions or
     * OS reinstall partitions on some systems.
     *
     * @return true if the volume should be ignored
     */
    virtual bool isIgnored() const = 0;

    /**
     * Retrieves the type of use for this volume (for example filesystem).
     *
     * @return the usage type
     * @see Solid::StorageVolume::UsageType
     */
    virtual Solid::StorageVolume::UsageType usage() const = 0;

    /**
     * Retrieves the filesystem type of this volume.
     *
     * @return the filesystem type if applicable, QString() otherwise
     */
    virtual QString fsType() const = 0;

    /**
     * Retrieves this volume label.
     *
     * @return the volume level if available, QString() otherwise
     */
    virtual QString label() const = 0;

    /**
     * Retrieves this volume Universal Unique IDentifier (UUID).
     *
     * You can generally assume that this identifier is unique with reasonable
     * confidence. Except if the volume UUID has been forged to intentionally
     * provoke a collision, the probability to have two volumes having the same
     * UUID is low.
     *
     * @return the Universal Unique IDentifier if available, QString() otherwise
     */
    virtual QString uuid() const = 0;

    /**
     * Retrieves this volume size in bytes.
     *
     * @return the size of this volume
     */
    virtual qulonglong size() const = 0;

    /**
     * Retrieves the crypto container UDI of this volume.
     *
     * @return the encrypted volume UDI containing the current volume if applicable,
     * an empty string otherwise
     */
    virtual QString encryptedContainerUdi() const = 0;
};
}
}

Q_DECLARE_INTERFACE(Solid::Ifaces::StorageVolume, "org.kde.Solid.Ifaces.StorageVolume/0.1")

#endif // SOLID_IFACES_STORAGEVOLUME_H
