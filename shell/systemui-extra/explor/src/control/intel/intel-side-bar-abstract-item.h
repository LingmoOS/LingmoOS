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

#ifndef INTEL_SIDEBARABSTRACTITEM_H
#define INTEL_SIDEBARABSTRACTITEM_H

#include <QObject>
#include <QVector>

#include "explor-core_global.h"
#include <gio/gio.h>

namespace Peony {

namespace Intel {

class SideBarModel;

class PEONYCORESHARED_EXPORT SideBarAbstractItem : public QObject
{
    friend class SideBarFileSystemItem;
    friend class SideBarModel;
    Q_OBJECT
public:
    enum Type {
        FavoriteItem,
        PersonalItem,
        FileSystemItem,
        SeparatorItem,
        VFSItem
    };

    explicit SideBarAbstractItem(SideBarModel* model, QObject *parent = nullptr);
    virtual ~SideBarAbstractItem();

    virtual Type type() = 0;

    virtual QString uri() = 0;
    virtual QString displayName() = 0;
    virtual QString iconName() = 0;
    virtual bool hasChildren() = 0;
    virtual bool isRemoveable() = 0;
    virtual bool isEjectable() = 0;
    virtual bool isMountable() = 0;

    virtual bool isMounted() {
        return false;
    }

    virtual QModelIndex firstColumnIndex() = 0;
    virtual QModelIndex lastColumnIndex() = 0;

    virtual SideBarAbstractItem *parent() = 0;

protected:
    QVector<SideBarAbstractItem*> *m_children = nullptr;
    SideBarModel *m_model = nullptr;
    QString m_unix_device; // sdb1, etc...
    QString m_volume_name; // Windows, Data etc...

Q_SIGNALS:
    void findChildrenFinished();
    void updated();

public Q_SLOTS:
    virtual void onUpdated() = 0;

    virtual void eject(GMountUnmountFlags ejectFlag) = 0;
    virtual void unmount() = 0;
    virtual void format() = 0;

    virtual void ejectOrUnmount() {}

    virtual void findChildren() = 0;
    virtual void findChildrenAsync() = 0;
    virtual void clearChildren();
};

}

}

#endif // SIDEBARABSTRACTITEM_H
