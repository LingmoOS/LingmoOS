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

#ifndef SIDEBARFILESYSTEMITEM_H
#define SIDEBARFILESYSTEMITEM_H

#include "explor-core_global.h"
#include "side-bar-abstract-item.h"
#include <memory>
#include <gio/gio.h>
#include "volumeManager.h"
#include "gerror-wrapper.h"

namespace Peony {

class FileWatcher;
class FileEnumerator;
class FileUtils;
class fstabData;
class PEONYCORESHARED_EXPORT SideBarFileSystemItem : public SideBarAbstractItem
{
    Q_OBJECT
public:
    explicit SideBarFileSystemItem(QString uri,const Experimental_Peony::Volume& volume,
                                   SideBarFileSystemItem *parentItem,
                                   SideBarModel *model,
                                   QObject *parent = nullptr);

    ~SideBarFileSystemItem();

    Type type() override {
        return SideBarAbstractItem::FileSystemItem;
    }

    QString uri() override;
    QString displayName() override;
    QString iconName() override {
        return m_iconName;
    }
    bool hasChildren() override {
        return true;
    }

    bool isRemoveable() override;
    bool isEjectable() override;
    bool isMountable() override;
    bool isUnmountable() override;
    //TODO: monitoring the mount state
    bool isMounted() override;

    QModelIndex firstColumnIndex() override;
    QModelIndex lastColumnIndex() override;

    SideBarAbstractItem *parent() override {
        return m_parent;
    }
    Experimental_Peony::Volume getVolume(){
        return m_volume;
    }

    bool filterShowRow();
    QString getDeviceUUID(const char *device);

private:
    void initDirInfo(const QString& uri);           //普通目录
    void initComputerInfo();                        //计算机Computer
    void initVolumeInfo(const Experimental_Peony::Volume& volumeItem);  //分区设备

public Q_SLOTS:    
    void eject(GMountUnmountFlags ejectFlag) override;    
    void unmount() override;
    void ejectOrUnmount() override;
    void mount()override;
    void format() override {}

    void onUpdated() override {}

    void findChildren() override;
    void findChildrenAsync() override;
    void clearChildren() override;

    void slot_volumeDeviceAdd(const Experimental_Peony::Volume& addItem);/*设备增加：插入、关闭gparted */
    void slot_volumeDeviceRemove(const QString& removeDevice);/*设备移除需要匹配device或者mountPoint属性,移除：弹出、拔出、打开gparted */
    void slot_volumeDeviceMount(const Experimental_Peony::Volume& volume);/*设备挂载 */
    void slot_volumeDeviceUnmount(const QString& unmountDevice);/*设备卸载 */
    void slot_volumeDeviceUpdate(const Experimental_Peony::Volume& updateDevice,QString property);/*设备的属性更新：如重设卷标*/

    void slot_fileCreate(const QString& uri);
    void slot_fileDelete(const QString& uri);
    void slot_fileRename(const QString &oldUri, const QString &newUri);

    void slot_enumeratorPrepared(const std::shared_ptr<GErrorWrapper>& err, const QString& targetUri, bool critical);
    void slot_enumeratorFinish(bool successed);

protected:
    void updateFileInfo(SideBarFileSystemItem *pThis);

private:
    SideBarFileSystemItem *m_parent = nullptr;
    Experimental_Peony::Volume m_volume = nullptr;
    std::shared_ptr<FileWatcher> m_watcher = nullptr;
    FileEnumerator *m_enumerator = nullptr;

    bool m_isRootChild = false;

    QString m_unix_device; // sdb1, etc...
    QString m_volume_name; // Windows, Data etc...
};

}

#endif // SIDEBARFILESYSTEMITEM_H
