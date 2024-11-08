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
 * Authors: dingjing <dingjing@kylinos.cn>
 *
 */

#ifndef COMPUTERUSERSHAREITEM_H
#define COMPUTERUSERSHAREITEM_H

#include <gio/gio.h>
#include "abstract-computer-item.h"
#include <explorer-qt/volume-manager.h>
#include <explorer-qt/PeonyFileWatcher>

class ComputerUserShareItem : public AbstractComputerItem
{
    Q_OBJECT
    friend void query_file_info_async_callback(GFile *file, GAsyncResult *res, ComputerUserShareItem* p_this);
public:
    explicit ComputerUserShareItem(GVolume *volume, ComputerModel *model, AbstractComputerItem *parentNode, QObject *parent = nullptr);
    ~ComputerUserShareItem();

    void updateInfoAsync();

    Type itemType() override {return Volume;}
    const QString uri() override {return m_uri;}
    const QString displayName() override {return m_displayName;};
    const QIcon icon() override {return m_icon;};
    bool isMount() override {return true;};

    void findChildren() override {};

    void updateInfo() override {updateInfoAsync();}
    void check() override;

    bool canEject() override {return false;};
    void eject(GMountUnmountFlags) override {};
    bool canUnmount() override {return false;};
    void unmount(GMountUnmountFlags) override {};

    void mount() override {};

    QModelIndex itemIndex() override;

    qint64 usedSpace() override {return m_usedSpace;};
    qint64 totalSpace() override {return m_totalSpace;}

    bool isHidden() override {return m_isHidden;};

protected:


private:
    QString                             m_uri;
    QString                             m_vfs_uri;
    GFile*                              m_file;

    QString                             m_displayName;
    QIcon                               m_icon;
    qint64                              m_totalSpace = 0;
    qint64                              m_usedSpace = 0;

    bool                                m_isHidden = false;
    GCancellable                        *m_cancellable = nullptr;
};

#endif // COMPUTERUSERSHAREITEM_H
