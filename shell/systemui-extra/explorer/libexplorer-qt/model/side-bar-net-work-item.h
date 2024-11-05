/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2021, KylinSoft Co., Ltd.
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
 * Authors: Wenfei He <hewenfei@kylinos.cn>
 *
 */

#ifndef SIDEBARNETWORKITEM_H
#define SIDEBARNETWORKITEM_H

#include "explorer-core_global.h"
#include "side-bar-abstract-item.h"
#include "usershare-manager.h"
#include "volumeManager.h"

#include <QModelIndex>
#include <QThread>
#include <QHash>

namespace Peony {

class SideBarModel;
class SharedDirectorInfoThread;
class FileWatcher;
class FileEnumerator;

class PEONYCORESHARED_EXPORT SideBarNetWorkItem : public SideBarAbstractItem
{
    Q_OBJECT
public:

    explicit SideBarNetWorkItem(const QString &uri,
                                const QString &iconName,
                                const QString &displayName,
                                SideBarNetWorkItem *parentItem,
                                SideBarModel *model,
                                QObject *parent = nullptr);

    virtual ~SideBarNetWorkItem();

    Type type() override
    {
        return SideBarAbstractItem::NetWorkItem;
    }

    QString uri() override;

    QString displayName() override;

    QString iconName() override;

    bool hasChildren() override;

    bool isRemoveable() override{
        return true;
    }

    bool isEjectable() override{
        return false;
    }

    bool isMountable() override{
        return false;
    }

    bool isUnmountable() override;

    bool isMounted() override;

    QModelIndex firstColumnIndex() override;

    QModelIndex lastColumnIndex() override;

    SideBarAbstractItem *parent() override
    {
        return m_parentItem;
    }

    void onUpdated() override {};

    void eject(GMountUnmountFlags ejectFlag) override {};

    void unmount() override;
    void realUnmount();

    void format() override {};

    void ejectOrUnmount() override;

    void findChildren() override;

    void findChildrenAsync() override;

    void clearChildren() override;

    void getMountedServers();
    void findRemoteServers();
    void querySharedFolders();

private:
    void addItemForUri(const QString &uri, const QString &iconName, const QString &displayName, SideBarNetWorkItem *parentItem,
                      SideBarModel *model, bool isVolume = false, QObject *parent = nullptr);
    void removeItemForUri(const QString uri);


private Q_SLOTS:
    void slot_addSharedFolder(const ShareInfo& shareInfo, bool successed);
    void slot_deleteSharedFolder(const QString& originalPath, bool successed);
    void slot_updateRemoteServer(const QString& server, bool add);
    void slot_unmountedRemoteServerCallBack(const QString& server);
    void slot_serverMount(const Experimental_Peony::Volume &volume);

protected:
    void initWatcher();
    void startWatcher();
    void stopWatcher();

private:
    SideBarNetWorkItem *m_parentItem  = nullptr;
    std::shared_ptr<FileWatcher> m_watcher = nullptr;
    FileEnumerator* m_enumerator = nullptr;
    bool m_canDeleteServer = false; /* 用于区分删除和只卸载 */
};

class SharedDirectoryInfoThread : public QThread {
Q_OBJECT
public:
    explicit SharedDirectoryInfoThread();

protected:
    void run() override;


Q_SIGNALS:
    void querySharedInfoFinish(QHash<QString,QString> sharedFolderInfoMap);


};

}

#endif //SIDEBARNETWORKITEM_H
