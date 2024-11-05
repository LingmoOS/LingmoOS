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

#ifndef COMPUTERNETWORKITEM_H
#define COMPUTERNETWORKITEM_H

#include "abstract-computer-item.h"

#include <QObject>
#include <explorer-qt/PeonyFileWatcher>

class ComputerNetworkItem : public AbstractComputerItem
{
    Q_OBJECT
public:
    explicit ComputerNetworkItem(const QString &uri, ComputerModel *model, AbstractComputerItem *parentNode, QObject *parent = nullptr);
    ~ComputerNetworkItem();

    Type itemType() override {return Network;}
    const QString uri() override {return m_uri;}
    const QString displayName() override;
    const QIcon icon() override {return m_icon;}

    void findChildren() override;

    void updateInfo() override;

    QModelIndex itemIndex() override;

public Q_SLOTS:
    void reloadDirectory(const QString &uri);

    void onFileAdded(const QString &uri);
    void onFileRemoved(const QString &uri);
    void onFileChanged(const QString &uri);

protected:
    //enumeration
    static void enumerate_async_callback(GFile *file, GAsyncResult *res, ComputerNetworkItem *p_this);
    static void find_children_async_callback(GFileEnumerator *enumerator, GAsyncResult *res, ComputerNetworkItem *p_this);

    //info
    static void query_info_async_callback(GFile *file, GAsyncResult *res, ComputerNetworkItem *p_this);

private:
    QString m_uri;
    QString m_displayName;
    QIcon m_icon;

    GCancellable *m_cancellable;

    Peony::FileWatcher *m_watcher = nullptr;
};

#endif // COMPUTERNETWORKITEM_H
