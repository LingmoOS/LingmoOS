/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#ifndef VOLUME_MANAGER_H
#define VOLUME_MANAGER_H

#include "explor-core_global.h"

#include <QObject>
#include <gio/gio.h>
#include <memory>

#include <QMetaType>

namespace Peony {

/*!
 * \brief The Drive class
 * is the abtraction of GDrive of glib.
 */
class PEONYCORESHARED_EXPORT Drive
{
public:
    Drive();//do not use this constructor
    Drive(GDrive *drive, bool takeOver = false) {
        m_drive = drive;
        m_take_over = takeOver;
    }
    ~Drive()
    {
        //do not unref the handle, it is managed by GVolumeMonitor.
        if (m_take_over) {
            g_object_unref(m_drive);
        }
    }

    QString name();
    QString iconName();
    QString symbolicIconName();

    bool removable()
    {
        if (!m_drive)
            return false;
#if GLIB_CHECK_VERSION(2, 50, 0)
        return g_drive_is_removable(m_drive);
#else
        //FIXME: old glib does not have relative api.
        //should i implete it?
        return false;
#endif
    }

    GDrive *getGDrive()
    {
        return m_drive;
    }

private:
    GDrive *m_drive = nullptr;
    bool m_take_over = false;
};

/*!
 * \brief The Volume class
 * is the abstraction of GVolume in glib
 */
class PEONYCORESHARED_EXPORT Volume
{
public:
    Volume();//do not use this constructor
    Volume(GVolume *volume, bool takeOver = false)
    {
        m_volume = volume;
        m_take_over = takeOver;
    }

    ~Volume()
    {
        //do not unref the handle, it is managed by GVolumeMonitor.
        if (m_take_over)
            g_object_unref(m_volume);
    }

    QString name();
    QString iconName();
    QString symbolicIconName();

    /*!
     * \brief uuid
     * \return the UUID for volume or NULL if no UUID can be computed.
     */
    QString uuid()
    {
        char *uuid = g_volume_get_uuid(m_volume);
        QString value = uuid;
        if (uuid)
            g_free(uuid);
        return value;
    }

    /*!
     * \brief rootUri
     * <br>
     * Gets the activation root uri for a GVolume
     * if it is known ahead of mount time.
     * Returns NULL otherwise.
     * </br>
     * \return the activation root uri or null.
     */
    QString rootUri()
    {
        GFile *root = g_volume_get_activation_root(m_volume);
        if (!G_IS_FILE (root))
            return nullptr;
        char *uri = g_file_get_uri(root);
        QString value = uri;
        g_free(uri);
        return value;
    }

    GVolume *getGVolume()
    {
        return m_volume;
    }

private:
    GVolume *m_volume = nullptr;
    bool m_take_over = false;
};

/*!
 * \brief The Mount class
 * is the abstraction of GMount in glib.
 */
class PEONYCORESHARED_EXPORT Mount
{
public:
    Mount();//do not use this constructor
    Mount(GMount *mount, bool takeOver = false)
    {
        m_mount = mount;
        m_take_over = takeOver;
    }

    ~Mount()
    {
        //do not unref the handle, it is managed by GVolumeMonitor.
        if (m_take_over)
            g_object_unref(m_mount);
    }

    QString name();
    QString iconName();
    QString symbolicIconName();

    /*!
     * \brief uuid
     * \return the UUID for mount or NULL if no UUID can be computed.
     */
    QString uuid()
    {
        char *uuid = g_mount_get_uuid(m_mount);
        QString value = uuid;
        if (uuid)
            g_free(uuid);
        return value;
    }

    /*!
     * \brief rootUri
     * \return
     * \note As volume has mounted, the rootUri never return null.
     */
    QString rootUri()
    {
        GFile *root = g_mount_get_root(m_mount);
        char *uri = g_file_get_uri(root);
        QString value = uri;
        g_free(uri);
        return value;
    }

    GMount *getGMount()
    {
        return m_mount;
    }

private:
    GMount *m_mount = nullptr;
    bool m_take_over = false;
};

class PEONYCORESHARED_EXPORT VolumeManager : public QObject
{
    Q_OBJECT
public:
    static VolumeManager *getInstance();

    /*!
     * \brief getMountFromUri
     * \param Uri
     * \return a Mount represent a GMount for uri, null if not.
     * \details
     * GMount is returned only for user interesting locations, see GVolumeMonitor.
     * If the GFileIface for file does not have a mount, error will be set to
     * G_IO_ERROR_NOT_FOUND and NULL will be returned.
     */
    static std::shared_ptr<Mount> getMountFromUri(const QString &uri);

    /*!
     * \brief getVolumeFromUri
     * \param uri
     * \return a Volume represent a GVolume for uri, null if not.
     * \details
     * Volume is from a GMount with g_mount_get_volume().
     */
    static std::shared_ptr<Volume> getVolumeFromUri(const QString &uri);

    /*!
     * \brief getDriveFromUri
     * \param uri
     * \return a Drive represent a GDrive for uri, null if not.
     * \details
     * Drive is from a GMount with g_mount_get_drive().
     */
    static std::shared_ptr<Drive> getDriveFromUri(const QString &uri);

    static std::shared_ptr<Volume> getVolumeFromMount(const std::shared_ptr<Mount> &mount);
    static std::shared_ptr<Drive> getDriveFromMount(const std::shared_ptr<Mount> &mount);
    std::shared_ptr<Drive> getDriveFromSystemByPath(const QString &unixPath);
    static const char* getUnixDeviceFileFromMountPoint(const char* mountPoint);

Q_SIGNALS:
    /*!
     * \brief driveConnected
     * \param drive
     * \note moc will failed when sending a GDrive handle into signal/slot.
     * so I have to take it with a custom class.
     * This case is also happened in GVolme and GMount handle.
     */
    void driveConnected(const std::shared_ptr<Drive> &drive);
    void driveDisconnected(const std::shared_ptr<Drive> &drive);
    void volumeAdded(const std::shared_ptr<Volume> &volume);
    void volumeRemoved(const std::shared_ptr<Volume> &volume);
    void mountAdded(const std::shared_ptr<Mount> &mount);
    void mountRemoved(const std::shared_ptr<Mount> &mount);
    void volumeChanged(const std::shared_ptr<Volume> &volume);

    // internal signal
    void fileUnmounted(const QString &uri);

protected:
    static void drive_connected_callback(GVolumeMonitor *monitor,
                                         GDrive *drive,
                                         VolumeManager *p_this);

    static void drive_disconnected_callback(GVolumeMonitor *monitor,
                                            GDrive *drive,
                                            VolumeManager *p_this);

    static void volume_added_callback(GVolumeMonitor *monitor,
                                      GVolume *volume,
                                      VolumeManager *p_this);

    static void volume_removed_callback(GVolumeMonitor *monitor,
                                        GVolume *volume,
                                        VolumeManager *p_this);

    static void mount_added_callback(GVolumeMonitor *monitor,
                                     GMount *mount,
                                     VolumeManager *p_this);

    static void mount_removed_callback(GVolumeMonitor *monitor,
                                       GMount *mount,
                                       VolumeManager *p_this);

    static void volume_changed_callback(GVolumeMonitor *monitor, GVolume *volume, VolumeManager *p_this);

public Q_SLOTS:
    static void unmount(const QString &uri);

protected:
    static void unmount_cb(GFile *file,
                           GAsyncResult *result,
                           GError **error, QString *targetUri);

private:
    explicit VolumeManager(QObject *parent = nullptr);
    ~VolumeManager();

    GVolumeMonitor *m_volume_monitor = nullptr;
    gulong m_drive_connected_handle = 0;
    gulong m_drive_disconnected_handle = 0;
    gulong m_volume_added_handle = 0;
    gulong m_volume_removed_handle = 0;
    gulong m_mount_added_handle = 0;
    gulong m_mount_removed_handle = 0;
    gulong m_volume_changed_handle = 0;
};

}

Q_DECLARE_METATYPE(Peony::Drive)
Q_DECLARE_METATYPE(std::shared_ptr<Peony::Drive>)
Q_DECLARE_METATYPE(Peony::Volume)
Q_DECLARE_METATYPE(std::shared_ptr<Peony::Volume>)
Q_DECLARE_METATYPE(Peony::Mount)
Q_DECLARE_METATYPE(std::shared_ptr<Peony::Mount>)

#endif // VOLUMEMANAGER_H
