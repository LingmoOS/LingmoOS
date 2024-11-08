/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2021, KylinSoft Co., Ltd.
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

#include "side-bar-single-item.h"
#include "side-bar-model.h"
#include "file-info.h"
#include "file-info-job.h"

using namespace Peony;

SideBarSingleItem::SideBarSingleItem(const QString &uri, const QString &iconName, const QString &displayName, SideBarModel *model, QObject *parent) :
    SideBarAbstractItem(model, parent),
    m_uri(uri),
    m_icon_name(iconName),
    m_display_name(displayName)
{
    m_info = FileInfo::fromUri(m_uri);
}

SideBarSingleItem::SideBarSingleItem(const QString &uri, SideBarModel *model, QObject *parent) :
    SideBarAbstractItem(model, parent)
{
    m_uri = uri;
    m_info = FileInfo::fromUri(m_uri);
    if (m_info.get()->isEmptyInfo()) {
        FileInfoJob j(m_info);
        j.querySync();
    }
    m_display_name = m_info.get()->displayName();
    m_icon_name = m_info.get()->iconName();
}

QString SideBarSingleItem::uri()
{
    return m_uri;
}

QString SideBarSingleItem::displayName()
{
    return m_display_name;
}

QString SideBarSingleItem::iconName()
{
    return m_icon_name;
}

QModelIndex SideBarSingleItem::firstColumnIndex()
{
    return m_model->firstColumnIndex(this);
}

QModelIndex SideBarSingleItem::lastColumnIndex()
{
    return m_model->lastColumnIndex(this);
}
