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

#ifndef TABLETSIDEBARFACTORY_H
#define TABLETSIDEBARFACTORY_H

#include "side-bar-plugin-iface.h"

namespace Peony {

namespace Intel {

class TabletSideBarFactory : public QObject, public SideBarPluginInterface
{
    Q_OBJECT
public:
    explicit TabletSideBarFactory(QObject *parent = nullptr);

    const QStringList keys();
    SideBar *create(QWidget *parent);

    PluginType pluginType();

    const QString name();
    const QString description();
    const QIcon icon();
    void setEnable(bool enable);
    bool isEnable();
};

}

}

#endif // TABLETSIDEBARFACTORY_H
