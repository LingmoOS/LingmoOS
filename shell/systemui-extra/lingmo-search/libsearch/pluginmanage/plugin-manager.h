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
#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include <QObject>
#include <QDebug>
#include <QDir>
#include <QPluginLoader>
#include <QApplication>
#include "plugin-iface.h"
#include "search-plugin-iface.h"
#include "search-task-plugin-iface.h"

namespace LingmoUISearch {
class LIBSEARCH_EXPORT PluginManager : public QObject
{
    Q_OBJECT
public:
    static void init();
    static PluginManager *getInstance();
    void close();

Q_SIGNALS:
    void pluginStateChanged(const QString &pluginName, bool enable);

private:
    explicit PluginManager(QObject *parent = nullptr);

    ~PluginManager();

};
}

#endif // PLUGINMANAGER_H
