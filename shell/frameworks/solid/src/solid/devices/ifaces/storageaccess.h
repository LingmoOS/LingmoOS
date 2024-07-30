/*
    SPDX-FileCopyrightText: 2007 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_IFACES_STORAGEACCESS_H
#define SOLID_IFACES_STORAGEACCESS_H

#include <solid/devices/ifaces/deviceinterface.h>
#include <solid/storageaccess.h>

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
class StorageAccess : virtual public DeviceInterface
{
public:
    /**
     * Destroys a StorageVolume object.
     */
    ~StorageAccess() override;

    /**
     * Indicates if this volume is mounted.
     *
     * @return true if the volume is mounted
     */
    virtual bool isAccessible() const = 0;

    /**
     * Retrieves the absolute path of this volume mountpoint.
     *
     * @return the absolute path to the mount point if the volume is
     * mounted, QString() otherwise
     */
    virtual QString filePath() const = 0;

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
     * Checks if source of the storage is encrypted.
     *
     * @return true if storage is encrypted one
     */
    virtual bool isEncrypted() const = 0;

    /**
     * Mounts the volume.
     *
     * @return the job handling the operation
     */
    virtual bool setup() = 0;

    /**
     * Unmounts the volume.
     *
     * @return the job handling the operation
     */
    virtual bool teardown() = 0;

    /**
     * Indicates if this volume can check for filesystem errors.
     *
     * @return true if the volume is can be checked
     */
    virtual bool canCheck() const;

    /**
     * Checks the filesystem for consistency avoiding any modifications or repairs.
     *
     * Mounted or unsupported filesystems will result in an error.
     *
     * @return Whether the filesystem is undamaged.
     */
    virtual bool check();

    /**
     * Indicates if this volume can repair filesystem errors.
     *
     * @return true if the volume is can be repaired
     */
    virtual bool canRepair() const;

    /**
     * Tries to repair the filesystem.
     *
     * Mounted or unsupported filesystems will result in an error.
     *
     * @return Whether the filesystem could be successfully repaired
     */
    virtual bool repair();

protected:
    // Q_SIGNALS:
    /**
     * This signal is emitted when the mount state of this device
     * has changed.
     *
     * @param newState true if the volume is mounted, false otherwise
     * @param udi the UDI of the volume
     */
    virtual void accessibilityChanged(bool accessible, const QString &udi) = 0;

    /**
     * This signal is emitted when the mount state of this device
     * has changed.
     *
     * @param newState true if the volume is mounted, false otherwise
     * @param udi the UDI of the volume
     */
    virtual void setupDone(Solid::ErrorType error, QVariant resultData, const QString &udi) = 0;

    /**
     * This signal is emitted when the mount state of this device
     * has changed.
     *
     * @param newState true if the volume is mounted, false otherwise
     * @param udi the UDI of the volume
     */
    virtual void teardownDone(Solid::ErrorType error, QVariant resultData, const QString &udi) = 0;

    /**
     * This signal is emitted when a setup of this device is requested.
     * The signal might be spontaneous i.e. it can be triggered by
     * another process.
     *
     * @param udi the UDI of the volume
     */
    virtual void setupRequested(const QString &udi) = 0;

    /**
     * This signal is emitted when a teardown of this device is requested.
     * The signal might be spontaneous i.e. it can be triggered by
     * another process
     *
     * @param udi the UDI of the volume
     */
    virtual void teardownRequested(const QString &udi) = 0;

    /**
     * This signal is emitted when a repair of this device is requested.
     * The signal might be spontaneous i.e. it can be triggered by
     * another process.
     *
     * @param udi the UDI of the volume
     */
    virtual void repairRequested(const QString &udi);

    /**
     * This signal is emitted when the attempted repaired of this
     * device is completed.
     *
     * @param error type of error that occurred, if any
     * @param errorData more information about the error, if any
     * @param udi the UDI of the volume
     */
    virtual void repairDone(Solid::ErrorType error, QVariant resultData, const QString &udi);
};
}
}

Q_DECLARE_INTERFACE(Solid::Ifaces::StorageAccess, "org.kde.Solid.Ifaces.StorageAccess/0.1")

#endif
