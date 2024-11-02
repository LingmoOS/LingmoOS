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

#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include <QObject>
#include "explor-core_global.h"

#include "plugin-iface.h"

namespace Peony {

/*!
 * \brief The PluginManager class
 * \details
 * This class is used to manage plugins of explor.
 *
 * \todo
 * add gui.
 */
class PEONYCORESHARED_EXPORT PluginManager : public QObject
{
    Q_OBJECT
public:
    static void init();
    static PluginManager *getInstance();
    void close();
    PluginInterface* getPluginByFileName(QString &fileName);
    void registerPlugin(PluginInterface* piface, QObject* plugin);

Q_SIGNALS:
    void pluginStateChanged(const QString &pluginName, bool enable);

public Q_SLOTS:
    void setPluginEnableByName(const QString &pluginName, bool enable);

private:
    explicit PluginManager(QObject *parent = nullptr);
    ~PluginManager();

    QHash<QString, PluginInterface*> m_hash;
    QHash<QString, PluginInterface*> m_fileNameHash;
};

}

#endif // PLUGINMANAGER_H
