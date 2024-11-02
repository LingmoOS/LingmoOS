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

#ifndef SIDEBARPERSONALITEM_H
#define SIDEBARPERSONALITEM_H

#include "explor-core_global.h"
#include "side-bar-abstract-item.h"

namespace Peony {

class SideBarModel;
class PEONYCORESHARED_EXPORT SideBarPersonalItem : public SideBarAbstractItem
{
    Q_OBJECT
public:
    explicit SideBarPersonalItem(QString uri,
                                 SideBarPersonalItem *parentItem,
                                 SideBarModel *model,
                                 QObject *parent = nullptr);

    Type type() override {
        return SideBarAbstractItem::PersonalItem;
    }

    QString uri() override;
    QString displayName();
    QString iconName();
    bool hasChildren() override {
        return m_is_root_child;
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

    QModelIndex firstColumnIndex() override;
    QModelIndex lastColumnIndex() override;

    SideBarAbstractItem *parent() override {
        return m_parent;
    }

public Q_SLOTS:
    void eject(GMountUnmountFlags ejectFlag) override {}
    void unmount() override {}
    void format() override {}

    void onUpdated() override {}

    void findChildren() override {}
    void findChildrenAsync() override {}
    void clearChildren() override {}

private:
    SideBarPersonalItem *m_parent = nullptr;

    bool m_is_root_child = false;
    QString m_uri = nullptr;
    QString m_display_name = nullptr;
    QString m_icon_name = nullptr;
};

}

#endif // SIDEBARPERSONALITEM_H
