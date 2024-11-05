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
 * Authors: Wang Weinan <wangweinan@kylinos.cn>
 *
 */


#ifndef SIEDBARCLOUDITEM_H
#define SIEDBARCLOUDITEM_H

#include "explorer-core_global.h"
#include "side-bar-abstract-item.h"

namespace Peony {

class SideBarModel;
class PEONYCORESHARED_EXPORT SideBarCloudItem : public SideBarAbstractItem
{
    Q_OBJECT
public:
    explicit SideBarCloudItem(QString uri,
                              SideBarCloudItem *parentItem,
                              SideBarModel *model,
                              QObject *parent = nullptr);

    Type type() override {
        return SideBarAbstractItem::PersonalItem;
    }

    QString uri() override {
        return m_uri;
    }
    QString displayName() override {
        return m_display_name;
    }
    QString iconName() override {
        return m_icon_name;
    }
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
    SideBarCloudItem *m_parent = nullptr;

    bool m_is_root_child = false;
    QString m_uri = nullptr;
    QString m_display_name = nullptr;
    QString m_icon_name = nullptr;
};

}
#endif // SIEDBARCLOUDITEM_H
