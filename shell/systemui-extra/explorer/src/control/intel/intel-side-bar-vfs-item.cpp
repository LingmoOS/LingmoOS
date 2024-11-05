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

#include "intel-side-bar-vfs-item.h"
#include "vfs-plugin-iface.h"
#include "vfs-plugin-manager.h"
#include "file-utils.h"

#include "intel-side-bar-model.h"

using namespace Peony::Intel;

SideBarVFSItem::SideBarVFSItem(VFSPluginIface *plugin, SideBarModel *model, QObject *parent) : SideBarAbstractItem(model, parent)
{
    m_plugin = plugin;
    m_model = model;
}

QString SideBarVFSItem::uri()
{
    if (m_uri.isEmpty()) {
        m_uri = m_plugin->uriScheme() + "/";
    }
    return m_uri;
}

QString SideBarVFSItem::displayName()
{
    if (m_display_name.isEmpty()) {
        auto vfsFile = VFSPluginManager::getInstance()->newVFSFile(uri());
        auto info = g_file_query_info(vfsFile, G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME, G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, nullptr, nullptr);
        m_display_name = g_file_info_get_display_name(info);
        g_object_unref(info);
        g_object_unref(vfsFile);
    }
    return m_display_name;
}

QString SideBarVFSItem::iconName()
{
    return m_plugin->icon().name();
}

QModelIndex SideBarVFSItem::firstColumnIndex()
{
    return m_model->firstColumnIndex(this);
}

QModelIndex SideBarVFSItem::lastColumnIndex()
{
    return m_model->lastColumnIndex(this);
}
