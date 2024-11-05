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

#ifndef INTEL_SIDEBARFILESYSTEMITEM_H
#define INTEL_SIDEBARFILESYSTEMITEM_H

#include "explorer-core_global.h"
#include "intel-side-bar-abstract-item.h"
#include <memory>
#include <gio/gio.h>

namespace Peony {

class FileWatcher;
class FileInfo;

namespace Intel {

class PEONYCORESHARED_EXPORT SideBarFileSystemItem : public SideBarAbstractItem
{
    Q_OBJECT
public:
    explicit SideBarFileSystemItem(QString uri,
                                   SideBarFileSystemItem *parentItem,
                                   SideBarModel *model,
                                   QObject *parent = nullptr);

    Type type() override {
        return SideBarAbstractItem::FileSystemItem;
    }

    QString uri() override {
        return m_uri;
    }
    QString displayName() override;
    QString iconName() override {
        return m_icon_name;
    }
    bool hasChildren() override {
        return true;
    }

    bool isRemoveable() override;
    bool isEjectable() override;
    bool isMountable() override;

    //TODO: monitoring the mount state
    bool isMounted() override;

    QModelIndex firstColumnIndex() override;
    QModelIndex lastColumnIndex() override;

    SideBarAbstractItem *parent() override {
        return m_parent;
    }

public Q_SLOTS:
    void eject(GMountUnmountFlags ejectFlag) override;
    void realEject(GMountUnmountFlags ejectFlag);
    void unmount() override;
    void realUnmount();
    void format() override {}

    void ejectOrUnmount() override;

    void onUpdated() override {}

    void findChildren() override;
    void findChildrenAsync() override;
    void clearChildren() override;

protected:
    void initWatcher();
    void startWatcher();
    void stopWatcher();

    static GAsyncReadyCallback eject_cb(GFile *file,
                                        GAsyncResult *res,
                                        SideBarFileSystemItem *p_this);
    void updateFileInfo(SideBarFileSystemItem *pThis);
    static void ejectDevicebyDrive(GObject* object,GAsyncResult* res,SideBarFileSystemItem *pThis);

private:
    SideBarFileSystemItem *m_parent = nullptr;

    bool m_is_root_child = false;
    QString m_uri = nullptr;
    QString m_display_name = nullptr;
    QString m_icon_name = nullptr;

    bool m_is_removeable = false;
    bool m_is_ejectable = false;
    bool m_is_mountable = false;

    bool m_is_mounted = false;
    std::shared_ptr<FileWatcher> m_watcher = nullptr;

    std::shared_ptr<FileInfo> m_info = nullptr;
};

class SideBarUserDiskItem : public SideBarFileSystemItem
{
    Q_OBJECT
public:
    explicit SideBarUserDiskItem(SideBarFileSystemItem *item, SideBarModel *model, QObject *parent = nullptr);
    Type type() override {
        return SideBarAbstractItem::FileSystemItem;
    }

    QString uri() override;
    QString displayName() override;
    QString iconName();
    bool hasChildren() override {
        return true;
    }

    bool isRemoveable() {return false;}
    bool isEjectable() {return false;}
    bool isMountable() {return false;}

    //TODO: monitoring the mount state
    bool isMounted() override {return true;}

    SideBarAbstractItem *parent() override;
public Q_SLOTS:
    void eject(GMountUnmountFlags ejectFlag) override {}
    void unmount() override {}
    void format() override {}

    void ejectOrUnmount() override {}

    void onUpdated() override {}

    void findChildren() override;
    void findChildrenAsync() override;
    void clearChildren() override;

private:
    SideBarAbstractItem *m_parent = nullptr;
};

}

}

#endif // SIDEBARFILESYSTEMITEM_H
