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
#ifndef SEARCHTASKPLUGINMANAGER_H
#define SEARCHTASKPLUGINMANAGER_H

#include <QObject>
#include <QUuid>
#include <QMap>
#include <QMutex>
#include <QMutexLocker>
#include "search-task-plugin-iface.h"

namespace LingmoUISearch {

class ManagedPlugin : public QObject
{
    Q_OBJECT
public:
    explicit ManagedPlugin(QObject *parent) : QObject(parent) {}
    ~ManagedPlugin() override;

    inline SearchTaskPluginIface *internalPlugin(const SearchProperty::SearchType& searchType);
    inline SearchTaskPluginIface *externalPlugin(const QString& customType);

    bool insertInternalPlugin(const SearchProperty::SearchType& searchType, SearchTaskPluginIface* plugin);
    bool insertExternalPlugin(const QString& customType, SearchTaskPluginIface* plugin);

private:
    QMutex m_mutex;
    QMap<size_t, SearchTaskPluginIface*> m_internalPlugins;
    QMap<QString, SearchTaskPluginIface*> m_externalPlugin;
};

class SearchTaskPluginManager : public QObject
{
    Q_OBJECT
public:
    static SearchTaskPluginManager *getInstance();
    SearchTaskPluginIface *initPlugins(const QUuid&, SearchProperty::SearchType, const QString& customType = QString());
    bool startSearch(const QUuid&, SearchProperty::SearchType, const QString& customType = QString());
    void destroyPlugins(const QUuid& uuid);
    void registerPluginPath(const QString& customType, const QString& pluginPath);
    bool isSearching(const QUuid &uuid, SearchProperty::SearchType searchType, const QString& customType = QString());

private:
    explicit SearchTaskPluginManager(QObject *parent = nullptr);
    SearchTaskPluginIface *getPlugin(SearchProperty::SearchType searchType, const QString& customType = QString());

    //这里初衷是把内外部插件分开管理，内部插件可以增加枚举值，外部插件似乎只能用编写者自定义的字符串区分？
    QHash<size_t, SearchTaskPluginIface*> m_buildinPlugin;
    QHash<QString, SearchTaskPluginIface*> m_loadedPlugin;
    QMap<QString, QString> m_loadedPluginPath;
    QMap<QUuid, ManagedPlugin*> m_managedPlugins;

    QMutex m_mutex;
};
}

#endif // SEARCHTASKPLUGINMANAGER_H
