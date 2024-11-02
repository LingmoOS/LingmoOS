/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2021, KylinSoft Co., Ltd.
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

#ifndef SIDEBARFACTORYMANAGER_H
#define SIDEBARFACTORYMANAGER_H

#include "side-bar-plugin-iface.h"
#include <QMap>

#include <QObject>

namespace Peony {

class PEONYCORESHARED_EXPORT SideBarFactoryManager : public QObject
{
    Q_OBJECT
public:
    static SideBarFactoryManager *getInstance();

    void registerFactory(SideBarPluginInterface *factory);
    SideBarPluginInterface *getFactoryFromKey(const QString &key);

    SideBarPluginInterface *getFactoryFromPlatformName();

private:
    explicit SideBarFactoryManager(QObject *parent = nullptr);

    QMap<QString, SideBarPluginInterface*> m_plugins;
};

}

#endif // SIDEBARFACTORYMANAGER_H
