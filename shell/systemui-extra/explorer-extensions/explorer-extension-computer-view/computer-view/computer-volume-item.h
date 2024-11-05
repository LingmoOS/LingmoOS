/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, Tianjin LINGMO Information Technology Co., Ltd.
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

#ifndef COMPUTERVOLUMEITEM_H
#define COMPUTERVOLUMEITEM_H

#include "abstract-computer-item.h"
#include <explorer-qt/volume-manager.h>
#include <explorer-qt/PeonyFileWatcher>

class ComputerVolumeItem : public AbstractComputerItem
{
    Q_OBJECT
public:
    explicit ComputerVolumeItem(GVolume *volume, ComputerModel *model, AbstractComputerItem *parentNode, QObject *parent = nullptr);
    //explicit ComputerVolumeItem(const QString uri,ComputerModel *model,AbstractComputerItem *parentNode,QObject *parent = nullptr);
    ~ComputerVolumeItem();

    void updateInfoAsync();

    Type itemType() override {return Volume;}
    const QString uri() override {return m_uri;}
    const QString displayName() override;
    const QIcon icon() override;
    bool isMount() override;

    void findChildren() override;

    void updateInfo() override {updateInfoAsync();}
    void check() override;

    bool canEject() override;
    void eject(GMountUnmountFlags ejectFlag) override;
    bool canUnmount() override;
    void unmount(GMountUnmountFlags unmountFlag) override;

    void mount() override;

    QModelIndex itemIndex() override;

    qint64 usedSpace() override {return m_usedSpace;}
    qint64 totalSpace() override {return m_totalSpace;}

    bool isHidden() override;

    QString getDeviceUUID(const char *device);

protected:
    //monitor
    static void volume_changed_callback(GVolume *volume, ComputerVolumeItem *p_this);
    static void volume_removed_callback(GVolume *volume, ComputerVolumeItem *p_this);
    static void mount_changed_callback(GVolumeMonitor* volumeMonitor, GMount *gmount, ComputerVolumeItem *p_this);
    static void mount_added_callback(GVolumeMonitor* volumeMonitor, GMount *gmount, ComputerVolumeItem *p_this);

    //info
    static void qeury_info_async_callback(GFile *file, GAsyncResult *res, ComputerVolumeItem *p_this);
    static void query_root_info_async_callback(GFile *file, GAsyncResult *res, ComputerVolumeItem *p_this);

    //mount op
    static void mount_async_callback(GVolume *volume, GAsyncResult *res, ComputerVolumeItem *p_this);
    static void unmount_async_callback(GObject *object, GAsyncResult *res, ComputerVolumeItem *p_this);
    static void eject_async_callback(GObject *object, GAsyncResult *res, ComputerVolumeItem *p_this);
    static void stop_async_callback(GDrive *drive, GAsyncResult *res, ComputerVolumeItem *p_this);

    //watcher
    //comment gparted process code to fix duplicated volume issue, bug#41623
//    void collectInfoWhenGpartedOpen(QString uri);
//    void onFileAdded(const QString &uri);
//    void onFileRemoved(const QString &uri);

    //gparted
    //void findChildrenWhenGPartedOpen();
    //static void enumerate_async_callback(GFile *file, GAsyncResult *res, ComputerVolumeItem *p_this);
    //static void find_children_async_callback(GFileEnumerator *enumerator, GAsyncResult *res, ComputerVolumeItem *p_this);
    friend quint64 calcVolumeCapacity(ComputerVolumeItem* pThis);

private Q_SLOTS:
    void onVolumeAdded(const std::shared_ptr<Peony::Volume> volume);
    void updateBlockIcons();

private:
    QString m_uri;
    QString m_vfs_uri;

    std::shared_ptr<Peony::Volume> m_volume = nullptr;
    std::shared_ptr<Peony::Mount> m_mount = nullptr;
    GCancellable *m_cancellable = nullptr;
    GCancellable *m_tmpCancellable = nullptr;
    GVolumeMonitor *m_volumeMonitor = nullptr;

    gulong m_volumeChangedHandle = 0;    
    gulong m_volumeRemovedHandle = 0;
    gulong m_mountChangedHandle = 0;
    gulong m_mountAddedHandle = 0;

    //info
    QString m_displayName;
    QIcon m_icon;
    qint64 m_totalSpace = 0;
    qint64 m_usedSpace = 0;

    Peony::FileWatcher *m_watcher = nullptr;
    QString m_targetUri;

    bool m_isHidden = false;
};

#endif // COMPUTERVOLUMEITEM_H
