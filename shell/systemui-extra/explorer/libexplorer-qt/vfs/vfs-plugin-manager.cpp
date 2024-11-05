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

#include "vfs-plugin-manager.h"

#include "favorite-vfs-register.h"
#include "search-vfs-register.h"
#include "label-vfs-register.h"
#include "custom-vfs-register.h"
#include "vfs-info-plugin-manager.h"
#include <QDebug>

using namespace Peony;

static VFSPluginManager *global_instance = nullptr;

VFSPluginManager *VFSPluginManager::getInstance()
{
    if (!global_instance)
        global_instance = new VFSPluginManager;
    return global_instance;
}

void VFSPluginManager::registerPlugin(VFSPluginIface *plugin)
{
    if (m_support_schemes.contains(plugin->uriScheme()))
        return;

    plugin->initVFS();
    m_plugins<<plugin;
    m_support_schemes<<plugin->uriScheme();
    Q_EMIT updateVFSPlugin(plugin, true);
}

void VFSPluginManager::unregisterPlugin(VFSPluginIface *plugin)
{
    if (!m_support_schemes.contains(plugin->uriScheme()))
        return;

//    plugin->initVFS();
//    m_plugins.removeOne(plugin);
//    m_support_schemes.removeOne(plugin->uriScheme());
    Q_EMIT updateVFSPlugin(plugin, false);
}

QList<VFSPluginIface *> VFSPluginManager::registeredPlugins()
{
    return m_plugins;
}

GFile *VFSPluginManager::newVFSFile(const QString &uri)
{
    int index = -1;
    for (auto scheme : m_support_schemes) {
        if (uri.startsWith(scheme)) {
            index = m_support_schemes.indexOf(scheme);
        }
    }

    if (index >= 0) {
        auto plugin = m_plugins.at(index);
        return G_FILE(plugin->parseUriToVFSFile(uri));
    } else {
        return g_file_new_for_uri(uri.toUtf8().constData());
    }
}

VFSPluginManager::VFSPluginManager(QObject *parent) : QObject(parent)
{
    auto searchVFSPlugin = new SearchVFSInternalPlugin;
    registerPlugin(searchVFSPlugin);

    auto favoriteVFSPlugin = new FavoriteVFSInternalPlugin;
    registerPlugin(favoriteVFSPlugin);

    auto labelVFSPlugin = new LabelVFSInternalPlugin;
    registerPlugin(labelVFSPlugin);

    QStringList keys = VFSInfoPluginManager::getInstance()->getAllPluginKeys();
    for (auto key : keys) {
        auto testVFSPlugin = new CustomVFSInternalPlugin(key);
        registerPlugin(testVFSPlugin);
    }
}
