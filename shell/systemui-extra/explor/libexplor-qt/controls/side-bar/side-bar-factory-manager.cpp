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

#include "side-bar-factory-manager.h"

#ifdef LINGMO_COMMON
#include <lingmosdk/lingmo-com4cxx.h>
#endif

using namespace Peony;

static SideBarFactoryManager *global_instance = nullptr;

SideBarFactoryManager *SideBarFactoryManager::getInstance()
{
    if (!global_instance) {
        global_instance = new SideBarFactoryManager;
    }
    return global_instance;
}

void SideBarFactoryManager::registerFactory(SideBarPluginInterface *factory)
{
    auto keys = factory->keys();
    for (auto key : keys) {
        m_plugins.insert(key, factory);
    }
}

SideBarPluginInterface *SideBarFactoryManager::getFactoryFromKey(const QString &key)
{
    return m_plugins.value(key);
}


SideBarPluginInterface *SideBarFactoryManager::getFactoryFromPlatformName()
{
#ifdef LINGMO_COMMON
    return m_plugins.value(QString::fromStdString(KDKGetPrjCodeName()));
#else
    return nullptr;
#endif
}

SideBarFactoryManager::SideBarFactoryManager(QObject *parent) : QObject(parent)
{

}
