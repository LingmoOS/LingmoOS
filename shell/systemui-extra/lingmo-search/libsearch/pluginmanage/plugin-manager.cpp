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
#include "plugin-manager.h"
#include "search-plugin-manager.h"
#include "search-task-plugin-manager.h"

using namespace LingmoUISearch;

static PluginManager *global_instance = nullptr;
void PluginManager::init()
{
    PluginManager::getInstance();
}

PluginManager *PluginManager::getInstance()
{
    if (!global_instance) {
        global_instance = new PluginManager;
    }
    return global_instance;
}

PluginManager::PluginManager(QObject *parent) : QObject(parent)
{
    QDir pluginsDir(PLUGIN_INSTALL_DIRS);
    pluginsDir.setFilter(QDir::Files);

    Q_FOREACH(QString fileName, pluginsDir.entryList(QDir::Files)) {
        QPluginLoader pluginLoader(pluginsDir.absoluteFilePath(fileName));
        // version check
        QString type = pluginLoader.metaData().value("MetaData").toObject().value("type").toString();
        QString version = pluginLoader.metaData().value("MetaData").toObject().value("version").toString();
        if(type == "SEARCH_PLUGIN") {
            if (version != SEARCH_PLUGIN_IFACE_VERSION) {
                qWarning() << "SEARCH_PLUGIN version check failed:" << fileName << "version:" << version << "iface version : " << SEARCH_PLUGIN_IFACE_VERSION;
                continue;
            }
        } else if(type == "SEARCH_TASK_PLUGIN") {
            if (version != SEARCH_TASK_PLUGIN_IFACE_VERSION) {
                qWarning() << "SEARCH_TASK_PLUGIN_IFACE_VERSION version check failed:" << fileName << "version:" << version << "iface version : " << SEARCH_TASK_PLUGIN_IFACE_VERSION;
                continue;
            }
        } else {
            qWarning() << "Unsupport plugin:" << fileName << "type:" << type;
            continue;
        }

        QObject *plugin = pluginLoader.instance();
        if (!plugin)
            continue;
        PluginInterface *piface = dynamic_cast<PluginInterface*>(plugin);
        if (!piface)
            continue;
        switch (piface->pluginType()) {
        case PluginInterface::PluginType::SearchPlugin: {
            auto p = dynamic_cast<SearchPluginIface *>(plugin);
            if(!SearchPluginManager::getInstance()->registerExternalPlugin(p, pluginsDir.absoluteFilePath(fileName))) {
                delete p;
            }
            break;
        }
        case PluginInterface::PluginType::SearchTaskPlugin: {
            auto p = dynamic_cast<SearchTaskPluginIface*>(plugin);
            SearchTaskPluginManager::getInstance()->registerPluginPath(p->getCustomSearchType(), pluginsDir.absoluteFilePath(fileName));
            delete p;
            break;
        }
        default:
            break;
        }
    }
}

PluginManager::~PluginManager()
{
}
