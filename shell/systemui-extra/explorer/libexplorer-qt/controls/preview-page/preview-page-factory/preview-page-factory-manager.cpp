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

#include "preview-page-factory-manager.h"
#include "default-preview-page-factory.h"

using namespace Peony;

static PreviewPageFactoryManager *globalInstance = nullptr;

PreviewPageFactoryManager *PreviewPageFactoryManager::getInstance()
{
    if (!globalInstance) {
        globalInstance = new PreviewPageFactoryManager;
    }
    return globalInstance;
}

PreviewPageFactoryManager::PreviewPageFactoryManager(QObject *parent) : QObject(parent)
{
    m_map = new QMap<QString, PreviewPagePluginIface*>();
    //load default and plugins.
    auto defaultFactory = DefaultPreviewPageFactory::getInstance();
    registerFactory(defaultFactory->name(), static_cast<PreviewPagePluginIface*>(defaultFactory));
    //registerFactory("test", static_cast<PreviewPagePluginIface*>(defaultFactory));
}

PreviewPageFactoryManager::~PreviewPageFactoryManager()
{
    if (m_map) {
        //FIXME: unload all module?
        delete m_map;
    }
}

const QStringList PreviewPageFactoryManager::getPluginNames()
{
    QStringList l;
    for (auto key : m_map->keys()) {
        l<<key;
    }
    return l;
}

bool PreviewPageFactoryManager::registerFactory(const QString &name, PreviewPagePluginIface *plugin)
{
    if (m_map->value(name)) {
        return false;
    }
    m_map->insert(name, plugin);
    return true;
}

bool PreviewPageFactoryManager::unregisterFactory(const QString &name, PreviewPagePluginIface *plugin)
{
    if (m_map->value(name)) {
        auto p = m_map->value(name);
        delete p;
        p = nullptr;
        m_map->remove(name);
        return true;
    }
    return false;
}

PreviewPagePluginIface *PreviewPageFactoryManager::getPlugin(const QString &name)
{
    m_last_preview_page_id = name;
    return m_map->value(name);
}

const QString PreviewPageFactoryManager::getLastPreviewPageId()
{
    if (m_last_preview_page_id.isNull()) {
        return m_map->firstKey();
    }
    return m_last_preview_page_id;
}
