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
#ifndef SEARCHPLUGINFACTORY_H
#define SEARCHPLUGINFACTORY_H

#include <QObject>
#include <QSettings>
#include "search-plugin-iface.h"
#include "plugin-info.h"

namespace LingmoUISearch {

class SearchPluginManager : public QObject
{
    Q_OBJECT

    QStringList m_defaultPluginOrder = {
        "Ai Search",
        "Applications Search",
        "Note Search",
        "Settings Search",
        "Dir Search",
        "File Search",
        "File Content Search",
        "Web Page"
    };

public:
    bool registerPlugin(SearchPluginIface *plugin);
    bool registerExternalPlugin(SearchPluginIface *plugin, const QString &path);
    bool reRegisterPlugin(SearchPluginIface *plugin, bool isExternal, const QString &externalPluginPath = QString());
    bool unregisterPlugin(SearchPluginIface *plugin, bool isExternal, const QString &externalPluginPath = QString());

    //测试用接口
    bool reRegisterPlugin(const QString &pluginName);
    bool unregisterPlugin(const QString &pluginName);
    bool changePluginPos(const QString &pluginName, int pos);

    static SearchPluginManager *getInstance();
    const QList<PluginInfo> getPluginIds();
    SearchPluginIface *getPlugin(const QString &pluginId);

private:
    explicit SearchPluginManager(QObject *parent = nullptr);
    ~SearchPluginManager();
    void initOrderSettings();

    std::map<QString, SearchPluginIface*> m_map;
    std::map<int, QString> m_plugin_order;//绑定plugin ID和优先级

    QSettings *m_orderSettings = nullptr;
    QStringList m_externalPlugins;

Q_SIGNALS:
    void unregistered(const QString& pluginName);
    void reRegistered(const QString& pluginName);
    void changePos(const QString& pluginName, int index);
};
}

#endif // SEARCHPLUGINFACTORY_H
