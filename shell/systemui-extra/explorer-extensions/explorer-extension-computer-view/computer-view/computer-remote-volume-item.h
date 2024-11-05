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

#ifndef COMPUTERREMOTEVOLUMEITEM_H
#define COMPUTERREMOTEVOLUMEITEM_H

#include <explorer-qt/PeonyFileWatcher>
#include <gio/gio.h>
#include "abstract-computer-item.h"

class ComputerRemoteVolumeItem : public AbstractComputerItem
{
    Q_OBJECT
public:
    explicit ComputerRemoteVolumeItem(const QString &uri, ComputerModel *model, AbstractComputerItem *parentNode, QObject *parent = nullptr);
    ~ComputerRemoteVolumeItem();

    Type itemType() override {return RemoteVolume;}
    const QString uri() override {return m_uri;}
    const QString displayName() override;
    const QIcon icon() override;

    void findChildren() override;

    void updateInfo() override;

    bool canUnmount() override {return true;}
    void unmount(GMountUnmountFlags unmountFlag) override;

    QModelIndex itemIndex() override;

    bool isHidden() override;

public:
    void onFileAdded(const QString &uri);
    void onFileRemoved(const QString &uri);
    void onFileChanged(const QString &uri);

protected:
    //enumeration
    static void enumerate_async_callback(GFile *file, GAsyncResult *res, ComputerRemoteVolumeItem *p_this);
    static void find_children_async_callback(GFileEnumerator *enumerator, GAsyncResult *res, ComputerRemoteVolumeItem *p_this);

    //info
    static void query_info_async_callback(GFile *file, GAsyncResult *res, ComputerRemoteVolumeItem *p_this);

    //monitor
private:
    QString m_uri;
    GCancellable *m_cancellable = nullptr;
    bool m_isUnixDevice = false;
    bool m_isHidden = true;

    QString m_displayName;
    QIcon m_icon;

    Peony::FileWatcher *m_watcher = nullptr;
};

#endif // COMPUTERREMOTEVOLUMEITEM_H
