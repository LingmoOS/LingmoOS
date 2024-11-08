/*
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
 */

#include "menu-extension-loader.h"
#include "menu-extension-plugin.h"

#include "menu/app-menu-plugin.h"
#include "favorite/favorite-extension-plugin.h"

#include <QDir>
#include <QPluginLoader>

namespace LingmoUIMenu {

QHash<QString, MenuExtensionPlugin*> MenuExtensionLoader::plugins = QHash<QString, MenuExtensionPlugin*>();

MenuExtensionLoader *MenuExtensionLoader::instance()
{
    static MenuExtensionLoader loader;
    return &loader;
}

MenuExtensionLoader::MenuExtensionLoader()
{
//    setBlackList({});
    load();
}

void MenuExtensionLoader::load()
{
    loadInternalExtension();
    loadExtensionFromDisk();

    expand();
}

void MenuExtensionLoader::loadInternalExtension()
{
    registerExtension(new FavoriteExtensionPlugin);
    registerExtension(new AppMenuPlugin);
}

void MenuExtensionLoader::loadExtensionFromDisk()
{
    QDir pluginsDir(LINGMO_MENU_EXTENSION_DIR);
    for(const QString& fileName : pluginsDir.entryList({"*.so"},QDir::Files)) {
        QPluginLoader pluginLoader(pluginsDir.absoluteFilePath(fileName));
        QJsonObject metaData = pluginLoader.metaData().value("MetaData").toObject();
        QString type = metaData.value("Type").toString();
        QString version = metaData.value("Version").toString();
        if(type != LINGMO_MENU_EXTENSION_I_FACE_TYPE) {
            continue;
        }

        if(version != LINGMO_MENU_EXTENSION_I_FACE_VERSION) {
            qWarning() << "LINGMO_MENU_EXTENSION version check failed:" << fileName << "version:" << version << "iface version : " << LINGMO_MENU_EXTENSION_I_FACE_VERSION;
            continue;
        }

        QObject *obj = pluginLoader.instance();
        if (!obj) {
            continue;
        }

        MenuExtensionPlugin *plugin = qobject_cast<MenuExtensionPlugin*>(obj);
        if (!plugin) {
            continue;
        }

        registerExtension(plugin);
    }
}

void MenuExtensionLoader::expand()
{
    for (const auto &plugin : MenuExtensionLoader::plugins) {
//        qDebug() << "Expand Extension:" << plugin->id();
        WidgetExtension *widget = plugin->createWidgetExtension();
        if (widget) {
            // register widget.
            m_widgets.append(widget);
        }

        ContextMenuExtension *contextMenu = plugin->createContextMenuExtension();
        if (contextMenu) {
            // 注册菜单
            m_menus.append(contextMenu);
        }
    }

    std::sort(m_widgets.begin(), m_widgets.end(), [] (const WidgetExtension *a, const WidgetExtension *b) {
        if (a->index() < 0) {
            return false;
        } else if (b->index() < 0) {
            return true;
        }
        return a->index() <= b->index();
    });

    std::sort(m_menus.begin(), m_menus.end(), [] (const ContextMenuExtension *a, const ContextMenuExtension *b) {
        if (a->index() < 0) {
            return false;
        } else if (b->index() < 0) {
            return true;
        }
        return a->index() <= b->index();
    });
}

void MenuExtensionLoader::setBlackList(const QStringList &blackList)
{
    m_blackList = blackList;
}

QList<WidgetExtension *> MenuExtensionLoader::widgets() const
{
    return m_widgets;
}

QList<ContextMenuExtension *> MenuExtensionLoader::menus() const
{
    return m_menus;
}

void MenuExtensionLoader::registerExtension(MenuExtensionPlugin *plugin)
{
    QString id = plugin->id();
    if (m_blackList.contains(id) || MenuExtensionLoader::plugins.contains(id)) {
        delete plugin;
        return;
    }

    MenuExtensionLoader::plugins.insert(id, plugin);
}

} // LingmoUIMenu
