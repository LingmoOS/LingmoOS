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

#ifndef STABLEPROPERTIESWINDOWPLUGINMANAGER_H
#define STABLEPROPERTIESWINDOWPLUGINMANAGER_H

#include "explorer-core_global.h"
#include "properties-window-tab-iface.h"

#include <QMap>
#include <QMutex>

namespace Peony {

class PropertiesWindowTabPagePluginIface;

class PropertiesWindowPluginManager : public QObject
{
    friend class PropertiesWindowFactory;

Q_OBJECT

public:
    //static PropertiesWindowPluginManager* getInstance();

    void release();
    void setOpenFromDesktop();

    void setOpenFromPeony();

    bool registerFactory(PropertiesWindowTabPagePluginIface *factory);
    bool unregisterFactory(PropertiesWindowTabPagePluginIface *factory);

    const QStringList getFactoryNames();

    PropertiesWindowTabPagePluginIface *getFactory(const QString &id);
    explicit PropertiesWindowPluginManager(QObject *parent = nullptr);

    ~PropertiesWindowPluginManager() override;
private:


    QHash<QString, PropertiesWindowTabPagePluginIface *> m_factory_hash;
    QMap<int, QString> m_sorted_factory_map;
    QMutex m_mutex;
};

}

#endif // PROPERTIESWINDOW_H
