/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 */

#include "vfs-info-plugin-iface.h"
#include "vfs-info-plugin-manager.h"
#include "custom-vfs-register.h"
#include <QDebug>

using namespace Peony;

static VFSInfoPluginManager *global_instance = nullptr;

VFSInfoPluginManager *Peony::VFSInfoPluginManager::getInstance()
{
    if (!global_instance) {
        global_instance = new VFSInfoPluginManager;
    }
    return global_instance;
}

void VFSInfoPluginManager::registerPlugin(Peony::VFSInfoPluginIface *plugin)
{
    if (m_support_path.contains(plugin->pathScheme())) {
        return;
    }

    m_pluginsMap.insert(plugin->pathScheme(), plugin);
    m_support_path.append(plugin->pathScheme());
}

VFSInfoPluginIface *VFSInfoPluginManager::userSchemeGetPlugin(QString &scheme)
{
    if (m_support_path.contains(scheme)) {
        return m_pluginsMap.value(scheme);
    }
    return nullptr;
}

const QStringList VFSInfoPluginManager::getAllPluginKeys()
{
    return m_pluginsMap.keys();
}

VFSInfoPluginManager::VFSInfoPluginManager(QObject *parent)
    : QObject(parent)
{
#ifndef VFS_CUSTOM_PLUGIN
    auto custom = new CustomVFSInfoInernalPlugin;
    registerPlugin(custom);

    auto local2 = new LocalVFSInfoInternalPlugin2;
    registerPlugin(local2);
#endif
}

HanderTransfer::HanderTransfer(QObject *parent) : QObject(parent)
{

}
