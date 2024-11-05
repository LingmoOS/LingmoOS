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

#ifndef SIDEBARSEPARATORITEM_H
#define SIDEBARSEPARATORITEM_H

#include <QObject>
#include "explorer-core_global.h"
#include "side-bar-abstract-item.h"

namespace Peony {

/*!
 * \brief The SideBarSeparatorItem class
 * \details
 * This class is used to help side bar layout.
 * It also provide 3 types of separator, large, small and empty-file.
 * The large separator is used to make a space for top of sidebar.
 * The small separator is used to make a space for different type root item (favorite, personal and computer).
 * The empty-file separator is used as a indicator of a side bar file system item directory without any child item.
 */
class PEONYCORESHARED_EXPORT SideBarSeparatorItem : public SideBarAbstractItem
{
    Q_OBJECT
public:
    enum Details {
        Large,
        EmptyFile,
        Small
    };

    explicit SideBarSeparatorItem(Details type,
                                  SideBarAbstractItem *parentItem,
                                  SideBarModel *model,
                                  QObject *parent = nullptr);

    Details separatorType() {
        return m_type;
    }

    Type type() override {
        return SideBarAbstractItem::SeparatorItem;
    }

    QString uri() override {
        return nullptr;
    }
    QString displayName() override {
        return m_type==EmptyFile?tr("(No Sub Directory)"):nullptr;
    }
    QString iconName() override {
        return nullptr;
    }
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
    SideBarAbstractItem *m_parent = nullptr;
    Details m_type;
};

}

#endif // SIDEBARSEPARATORITEM_H
