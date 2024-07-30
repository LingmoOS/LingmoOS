/*
    SPDX-FileCopyrightText: 2006-2007 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_STORAGEACCESS_H
#define SOLID_STORAGEACCESS_H

#include <solid/solid_export.h>

#include <QVariant>
#include <solid/deviceinterface.h>
#include <solid/solidnamespace.h>

namespace Solid
{
class StorageAccessPrivate;
class Device;

/**
 * @class Solid::StorageAccess storageaccess.h <Solid/StorageAccess>
 *
 * This device interface is available on volume devices to access them
 * (i.e. mount or unmount them).
 *
 * A volume is anything that can contain data (partition, optical disc,
 * memory card). It's a particular kind of block device.
 */
class SOLID_EXPORT StorageAccess : public DeviceInterface
{
    Q_OBJECT
    Q_PROPERTY(bool accessible READ isAccessible)
    Q_PROPERTY(QString filePath READ filePath)
    Q_PROPERTY(bool ignored READ isIgnored)
    Q_PROPERTY(bool encrypted READ isEncrypted)
    Q_DECLARE_PRIVATE(StorageAccess)
    friend class Device;

private:
    /**
     * Creates a new StorageAccess object.
     * You generally won't need this. It's created when necessary using
     * Device::as().
     *
     * @param backendObject the device interface object provided by the backend
     * @see Solid::Device::as()
     */
    SOLID_NO_EXPORT explicit StorageAccess(QObject *backendObject);

public:
    /**
     * Destroys a StorageAccess object.
     */
    ~StorageAccess() override;

    /**
     * Get the Solid::DeviceInterface::Type of the StorageAccess device interface.
     *
     * @return the StorageVolume device interface type
     * @see Solid::Ifaces::Enums::DeviceInterface::Type
     */
    static Type deviceInterfaceType()
    {
        return DeviceInterface::StorageAccess;
    }

    /**
     * Indicates if this volume is mounted.
     *
     * @return true if the volume is mounted
     */
    bool isAccessible() const;

    /**
     * Retrieves the absolute path of this volume mountpoint.
     *
     * @return the absolute path to the mount point if the volume is
     * mounted, QString() otherwise
     */
    QString filePath() const;

    /**
     * Indicates if this volume should be ignored by applications.
     *
     * If it should be ignored, it generally means that it should be
     * invisible to the user. It's useful for firmware partitions or
     * OS reinstall partitions on some systems.
     *
     * @return true if the volume should be ignored
     */
    bool isIgnored() const;

    /**
     * Checks if source of the storage is encrypted.
     *
     * @return true if storage is encrypted one
     *
     * @since 5.80
     */
    bool isEncrypted() const;

    /**
     * Mounts the volume.
     *
     * @return false if the operation is not supported, true if the
     * operation is attempted
     */
    bool setup();

    /**
     * Unmounts the volume.
     *
     * @return false if the operation is not supported, true if the
     * operation is attempted
     */
    bool teardown();

    /**
     * Indicates if this volume can check for filesystem errors.
     *
     * @return true if the volume is can be checked
     */
    bool canCheck() const;

    /**
     * Checks the filesystem for consistency avoiding any modifications or repairs.
     *
     * Mounted or unsupported filesystems will result in an error.
     *
     * @return Whether the filesystem is undamaged.
     */
    bool check();

    /**
     * Indicates if the filesystem of this volume supports repair
     * attempts. It does not indicate if such an attempt will succeed.
     *
     * @return true if the volume is can be repaired
     */
    bool canRepair() const;

    /**
     * Tries to repair the filesystem.
     *
     * Mounted or unsupported filesystems will result in an error.
     *
     * @return Whether the filesystem could be successfully repaired
     */
    bool repair();

Q_SIGNALS:
    /**
     * This signal is emitted when the accessiblity of this device
     * has changed.
     *
     * @param accessible true if the volume is accessible, false otherwise
     * @param udi the UDI of the volume
     */
    void accessibilityChanged(bool accessible, const QString &udi);

    /**
     * This signal is emitted when the attempted setting up of this
     * device is completed. The signal might be spontaneous i.e.
     * it can be triggered by another process.
     *
     * @param error type of error that occurred, if any
     * @param errorData more information about the error, if any
     * @param udi the UDI of the volume
     */
    void setupDone(Solid::ErrorType error, QVariant errorData, const QString &udi);

    /**
     * This signal is emitted when the attempted tearing down of this
     * device is completed. The signal might be spontaneous i.e.
     * it can be triggered by another process.
     *
     * @param error type of error that occurred, if any
     * @param errorData more information about the error, if any
     * @param udi the UDI of the volume
     */
    void teardownDone(Solid::ErrorType error, QVariant errorData, const QString &udi);

    /**
     * This signal is emitted when a setup of this device is requested.
     * The signal might be spontaneous i.e. it can be triggered by
     * another process.
     *
     * @param udi the UDI of the volume
     */
    void setupRequested(const QString &udi);

    /**
     * This signal is emitted when a teardown of this device is requested.
     * The signal might be spontaneous i.e. it can be triggered by
     * another process
     *
     * @param udi the UDI of the volume
     */
    void teardownRequested(const QString &udi);

    /**
     * This signal is emitted when a repair of this device is requested.
     * The signal might be spontaneous i.e. it can be triggered by
     * another process.
     *
     * @param udi the UDI of the volume
     */
    void repairRequested(const QString &udi);

    /**
     * This signal is emitted when the attempted repaired of this
     * device is completed. The signal might be spontaneous i.e.
     * it can be triggered by another process.
     *
     * @param error type of error that occurred, if any
     * @param errorData more information about the error, if any
     * @param udi the UDI of the volume
     */
    void repairDone(Solid::ErrorType error, QVariant errorData, const QString &udi);

protected:
    /**
     * @internal
     */
    SOLID_NO_EXPORT StorageAccess(StorageAccessPrivate &dd, QObject *backendObject);
};
}

#endif
