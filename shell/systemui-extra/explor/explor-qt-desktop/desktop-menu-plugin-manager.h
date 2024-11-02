/*
 * Peony-Qt
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#ifndef DESKTOPMENUPLUGINMANAGER_H
#define DESKTOPMENUPLUGINMANAGER_H

#include <QObject>
#include <QMap>
#include "menu-plugin-iface.h"

namespace Peony {

class DesktopMenuPluginManager : public QObject
{
    Q_OBJECT
public:
    static DesktopMenuPluginManager *getInstance();
    bool isLoaded() {
        return m_is_loaded;
    }
    const QStringList getPluginIds();
    QList<MenuPluginInterface*> getPlugins();
    MenuPluginInterface *getPlugin(const QString &pluginId);
    void registerPlugin(const QString &uri);

protected:
    void loadAsync();

Q_SIGNALS:
    void pluginLoadFinished();

private:
    explicit DesktopMenuPluginManager(QObject *parent = nullptr);
    ~DesktopMenuPluginManager();

    QMap<QString, MenuPluginInterface*> m_map;
    bool m_is_loaded = false;
};

}

#endif // DESKTOPMENUPLUGINMANAGER_H
