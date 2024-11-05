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

#ifndef INTEL_SIDEBARVFSITEM_H
#define INTEL_SIDEBARVFSITEM_H

#include "explorer-core_global.h"
#include "intel-side-bar-abstract-item.h"

namespace Peony {

class VFSPluginIface;

namespace Intel {

class SideBarModel;

class PEONYCORESHARED_EXPORT SideBarVFSItem : public SideBarAbstractItem
{
    Q_OBJECT
public:
    explicit SideBarVFSItem(VFSPluginIface *plugin, SideBarModel *model, QObject *parent = nullptr);

    Type type() override {
        return SideBarAbstractItem::FileSystemItem;
    }

    QString uri() override;
    QString displayName() override;
    QString iconName() override;

    bool hasChildren() override {
        return false;
    }

    bool isRemoveable() override {
        return false;
    }
    bool isEjectable() override {
        return false;
    }
    bool isMountable() override {
        return false;
    }

    //TODO: monitoring the mount state
    bool isMounted() override {
        return false;
    }

    QModelIndex firstColumnIndex() override;
    QModelIndex lastColumnIndex() override;

    SideBarAbstractItem *parent() override {
        return nullptr;
    }

public Q_SLOTS:
    void eject(GMountUnmountFlags ejectFlag) override {}
    void unmount() override {}
    void format() override {}

    void ejectOrUnmount() override {}

    void onUpdated() override {}

    void findChildren() override {}
    void findChildrenAsync() override {}
    void clearChildren() override {}

private:
    VFSPluginIface *m_plugin;

    QString m_uri;
    QString m_display_name;
    SideBarModel *m_model;
};

}

}

#endif // SIDEBARVFSITEM_H
