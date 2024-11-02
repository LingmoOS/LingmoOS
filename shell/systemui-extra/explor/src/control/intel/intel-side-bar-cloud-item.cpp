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

#include "intel-side-bar-cloud-item.h"
#include "intel-side-bar-model.h"
#include "file-utils.h"
#include <QStandardPaths>

using namespace Peony::Intel;

SideBarCloudItem::SideBarCloudItem(QString uri,
        SideBarCloudItem *parentItem,
        SideBarModel *model,
        QObject *parent) : SideBarAbstractItem (model, parent)
{
    m_parent = parentItem;
    m_is_root_child = parentItem == nullptr;
    if (m_is_root_child) {
        QString homeUri = "cloud:///";
        m_uri = homeUri;
        m_display_name = tr("CloudStorage");
        //m_icon_name = "emblem-personal";
        //top dir don't show icon
        m_icon_name = "";

        QString documentUri = "file:///home/";
        SideBarCloudItem *documentItem = new SideBarCloudItem(documentUri,
                this,
                m_model);
        m_children->append(documentItem);

        m_model->insertRows(0, 5, firstColumnIndex());
        return;
    }
    m_uri = uri;
    m_display_name = tr("CloudFile");
    m_icon_name = "lingmo-cloud-file";// FileUtils::getFileIconName(uri);
}

QModelIndex SideBarCloudItem::firstColumnIndex()
{
    return m_model->firstColumnIndex(this);
}

QModelIndex SideBarCloudItem::lastColumnIndex()
{
    return m_model->lastColumnIndex(this);
}
