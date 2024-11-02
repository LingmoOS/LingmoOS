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

#ifndef SIDEBARABSTRACTITEM_H
#define SIDEBARABSTRACTITEM_H

#include <QObject>
#include <QVector>

#include "explor-core_global.h"
#include <gio/gio.h>

#include <memory>

namespace Peony {

class SideBarModel;
class FileInfo;

class PEONYCORESHARED_EXPORT SideBarAbstractItem : public QObject
{
    friend class SideBarFileSystemItem;
    friend class SideBarModel;
    friend class SideBarVFSItem;
    Q_OBJECT
public:
    enum Type {
        FavoriteItem,
        PersonalItem,
        FileSystemItem,
        NetWorkItem,
        SeparatorItem,
        VFSItem,
        SingleItem
    };

    explicit SideBarAbstractItem(SideBarModel* model, QObject *parent = nullptr);
    virtual ~SideBarAbstractItem();

    virtual Type type() = 0;

    virtual QString uri(){
        return m_uri;
    }
    virtual QString displayName(){
        return m_displayName;
    }
    virtual QString iconName(){
        return m_iconName;
    }
    virtual bool isRemoveable(){
        return m_removeable;
    }
    virtual bool isEjectable() {
        return m_ejectable;
    }
    virtual bool isStopable() {
        return m_stopable;
    }
    virtual bool isMountable() {
        return m_mountable;
    }
    virtual bool isUnmountable(){
        return m_unmountable;
	}
    virtual bool isMounted() {
        return m_mounted;
    }
    virtual QString getDevice() {
        return m_device;
    }
    virtual bool isVolume() {
        return m_isVolume;
    }
    virtual bool hasChildren() = 0;
    virtual QModelIndex firstColumnIndex();
    virtual QModelIndex lastColumnIndex();

    virtual SideBarAbstractItem *parent() = 0;

    virtual bool filterShowRow(){
        return !m_hidden;
    }

protected:
    QVector<SideBarAbstractItem*> *m_children = nullptr;
    SideBarModel *m_model = nullptr;
    std::shared_ptr<FileInfo> m_info;

    QString m_uri;
    QString m_displayName;
    QString m_iconName;

    QString m_device;
    QString m_mountPoint;

    bool m_removeable = false;
    bool m_ejectable = false;
    bool m_stopable = false;
    bool m_mountable = false;
    bool m_unmountable = false;
    bool m_mounted = false;
    bool m_isVolume = false;
    bool m_hidden = false;

Q_SIGNALS:
    void queryInfoFinished();
    void findChildrenFinished();
    void updated();

public Q_SLOTS:
    virtual void onUpdated(){}

    virtual void eject(GMountUnmountFlags ejectFlag){};
    virtual void unmount(){}
    virtual void format(){}

    virtual void ejectOrUnmount() {}
    virtual void mount(){}
    virtual void findChildren() = 0;
    virtual void findChildrenAsync() = 0;
    virtual void clearChildren();
};

}

#endif // SIDEBARABSTRACTITEM_H
