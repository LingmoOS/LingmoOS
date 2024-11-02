/*
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: iaom <zhangpengfei@kylinos.cn>
 */
#include "search-task-plugin-manager.h"
#include <QDebug>
#include <QPluginLoader>

#include "file-search-task.h"
#include "file-content-search-task.h"
#include "app-search-task.h"

using namespace LingmoUISearch;
static SearchTaskPluginManager *global_instance = nullptr;
SearchTaskPluginManager *SearchTaskPluginManager::getInstance()
{
    if (!global_instance) {
        global_instance = new SearchTaskPluginManager;
    }
    return global_instance;
}

SearchTaskPluginManager::SearchTaskPluginManager(QObject *parent) : QObject(parent)
{
}

bool SearchTaskPluginManager::startSearch(const QUuid &uuid, SearchProperty::SearchType searchType, const QString &customType)
{
    QMutexLocker locker(&m_mutex);
    if (m_managedPlugins.contains(uuid)) {
        ManagedPlugin* managedPlugin = m_managedPlugins.value(uuid);

        SearchTaskPluginIface *plugin = nullptr;
        if (searchType == SearchProperty::SearchType::Custom) {
            plugin = managedPlugin->externalPlugin(customType);

        } else {
            plugin = managedPlugin->internalPlugin(searchType);
        }

        if (plugin) {
            plugin->startSearch();
            return true;
        }
    }

    return false;
}

SearchTaskPluginIface *SearchTaskPluginManager::getPlugin(SearchProperty::SearchType searchType, const QString& customType)
{
    SearchTaskPluginIface *searchPlugin = nullptr;
    if (searchType == SearchProperty::SearchType::Custom) {
        if (m_loadedPluginPath.contains(customType)) {
            QPluginLoader pluginLoader(m_loadedPluginPath.value(customType));
            QObject *plugin = pluginLoader.instance();
            if (plugin) {
                searchPlugin = dynamic_cast<SearchTaskPluginIface*>(plugin);
            }
        }

    } else {
        switch (searchType) {
            case SearchProperty::SearchType::File:
                searchPlugin = new FileSearchTask();
                break;
            case SearchProperty::SearchType::FileContent:
                searchPlugin = new FileContentSearchTask();
                break;
            case SearchProperty::SearchType::Application:
                searchPlugin = new AppSearchTask();
                break;
            default:
                break;
        }
    }

    return searchPlugin;
}

void SearchTaskPluginManager::registerPluginPath(const QString& customType, const QString &pluginPath)
{
    QMutexLocker locker(&m_mutex);
    if (!m_loadedPluginPath.contains(customType)) {
        m_loadedPluginPath.insert(customType, pluginPath);
    }
}

bool SearchTaskPluginManager::isSearching(const QUuid &uuid, SearchProperty::SearchType searchType, const QString &customType)
{
    QMutexLocker locker(&m_mutex);
    if (m_managedPlugins.contains(uuid)) {
        ManagedPlugin* managedPlugin = m_managedPlugins.value(uuid);

        SearchTaskPluginIface *plugin = nullptr;
        if (searchType == SearchProperty::SearchType::Custom) {
            plugin = managedPlugin->externalPlugin(customType);

        } else {
            plugin = managedPlugin->internalPlugin(searchType);
        }

        if (plugin) {
            return plugin->isSearching();
        }
    }
    return false;
}

SearchTaskPluginIface *SearchTaskPluginManager::initPlugins(const QUuid& uuid, SearchProperty::SearchType searchType, const QString& customType)
{
    QMutexLocker locker(&m_mutex);
    if (!m_managedPlugins.contains(uuid)) {
        m_managedPlugins.insert(uuid, new ManagedPlugin(nullptr));
    }

    SearchTaskPluginIface *plugin = getPlugin(searchType, customType);
    bool succeed = false;
    if (searchType == SearchProperty::SearchType::Custom) {
        succeed = m_managedPlugins.value(uuid)->insertExternalPlugin(customType, plugin);
    } else {
        succeed = m_managedPlugins.value(uuid)->insertInternalPlugin(searchType, plugin);
    }

    return succeed ? plugin : nullptr;
}

void SearchTaskPluginManager::destroyPlugins(const QUuid &uuid)
{
    QMutexLocker locker(&m_mutex);
    delete m_managedPlugins.take(uuid);
}

ManagedPlugin::~ManagedPlugin()
{
    qDeleteAll(m_internalPlugins);
    qDeleteAll(m_externalPlugin);
}

bool ManagedPlugin::insertInternalPlugin(const SearchProperty::SearchType &searchType, SearchTaskPluginIface *plugin)
{
    QMutexLocker locker(&m_mutex);
    if (plugin) {
        auto type = static_cast<size_t>(searchType);
        plugin->setParent(this);

        if (!m_internalPlugins.contains(type)) {
            m_internalPlugins.insert(type, plugin);
            return true;
        }

        delete plugin;
    }
    return false;
}

bool ManagedPlugin::insertExternalPlugin(const QString &customType, SearchTaskPluginIface *plugin)
{
    QMutexLocker locker(&m_mutex);
    if (plugin) {
        plugin->setParent(this);
        if (!customType.isEmpty() && !m_externalPlugin.contains(customType)) {
            m_externalPlugin.insert(customType, plugin);
            return true;
        }

        delete plugin;
    }

    return false;
}

inline SearchTaskPluginIface *ManagedPlugin::internalPlugin(const SearchProperty::SearchType &searchType)
{
    QMutexLocker locker(&m_mutex);
    auto type = static_cast<size_t>(searchType);
    if (m_internalPlugins.contains(type)) {
        return m_internalPlugins.value(type);
    }
    return nullptr;
}

inline SearchTaskPluginIface *ManagedPlugin::externalPlugin(const QString &customType)
{
    QMutexLocker locker(&m_mutex);
    if (m_externalPlugin.contains(customType)) {
        return m_externalPlugin.value(customType);
    }
    return nullptr;
}
