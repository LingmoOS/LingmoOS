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
 * Authors: Yan Wei Yang <yangyanwei@kylinos.cn>
 *
 */

#ifndef PROPERTIESWINDOWFACTORYPLUGINMANAGER_H
#define PROPERTIESWINDOWFACTORYPLUGINMANAGER_H

#include <QObject>
#include <QMainWindow>
#include "explorer-core_global.h"

namespace Peony {

class PropertiesWindowFactoryPluginIface;
class PropertiesWindowTabPagePluginIface;

class PEONYCORESHARED_EXPORT PropertiesWindowFactoryPluginManager : public QObject
{
    friend class PropertiesWindowFactoryPluginIface;

Q_OBJECT

public:
    static PropertiesWindowFactoryPluginManager* getInstance();

    void release();
    bool loadFactory(PropertiesWindowFactoryPluginIface *factory);
    bool deleteFactory(PropertiesWindowFactoryPluginIface *factory);

    bool registerFactory(PropertiesWindowTabPagePluginIface *factory);
    bool unregisterFactory(PropertiesWindowTabPagePluginIface *factory);

    QMainWindow *create(const QStringList &uris);

    PropertiesWindowFactoryPluginIface *getFactory(const QString &id);

    explicit PropertiesWindowFactoryPluginManager(QObject *parent = nullptr);

    ~PropertiesWindowFactoryPluginManager() override;

private:
    QHash<QString, PropertiesWindowFactoryPluginIface *> m_factory_hash;
};
}

#endif // PLUGINMANAGER_H
