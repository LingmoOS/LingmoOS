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

#include "intel-side-bar-separator-item.h"
#include "intel-side-bar-model.h"

using namespace Peony::Intel;

SideBarSeparatorItem::SideBarSeparatorItem(Details type,
        SideBarAbstractItem *parentItem,
        SideBarModel *model,
        QObject *parent) : SideBarAbstractItem(model, parent)
{
    m_type = type;
    m_parent = parentItem;
}

QModelIndex SideBarSeparatorItem::firstColumnIndex()
{
    return m_model->firstColumnIndex(this);
}

QModelIndex SideBarSeparatorItem::lastColumnIndex()
{
    return m_model->lastColumnIndex(this);
}
