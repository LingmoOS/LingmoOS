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

#ifndef SIDEBARVFSITEM_H
#define SIDEBARVFSITEM_H

#include "explor-core_global.h"
#include "side-bar-abstract-item.h"
#include "gerror-wrapper.h"

namespace Peony {

class SideBarModel;
class FileEnumerator;
class FileWatcher;

class PEONYCORESHARED_EXPORT SideBarVFSItem : public SideBarAbstractItem
{
    Q_OBJECT
public:
    explicit SideBarVFSItem(const QString& uri, SideBarVFSItem* parentItem, SideBarModel *model, QObject *parent = nullptr);
    ~SideBarVFSItem();

    Type type() override {
        return SideBarAbstractItem::VFSItem;
    }

    QString uri() override;
    QString displayName() override;
    QString iconName() override;

    bool hasChildren() override {
        return true;
    }
    SideBarAbstractItem *parent() override {
        return m_parentItem;
    }

public Q_SLOTS:
    void findChildren() override;
    void findChildrenAsync() override;
    void clearChildren() override;
    void slot_enumeratorFinish(bool successed);/* 遍历完成 */
    void slot_fileCreate(const QString& uri);/* 增加 */
    void slot_fileDelete(const QString& uri);/* 删除 */
    void slot_fileSafeLocked(const QString& uri);/* 锁定 */
    void slot_fileSafeUpdate(const QString& uri);/* 用于文件保护箱解锁/锁定时更新icon */

private:
    SideBarVFSItem *m_parentItem = nullptr;
    FileEnumerator *m_enumerator = nullptr;
    std::shared_ptr<FileWatcher> m_watcher = nullptr;
};

}

#endif // SIDEBARVFSITEM_H
