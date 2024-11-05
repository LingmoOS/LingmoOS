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

#ifndef PROPERTIESWINDOWTABPAGEPLUGINIFACE_H
#define PROPERTIESWINDOWTABPAGEPLUGINIFACE_H

#include <QPluginLoader>
#include <QtPlugin>
#include <QStringList>
#include "plugin-iface.h"
#include "properties-window-tab-iface.h"

class QWidget;

#define PropertiesWindowTabPagePluginIface_iid "org.lingmo.explorer-qt.plugin-iface.PropertiesWindowTabPagePluginInterface"

namespace Peony {

class PropertiesWindowTabPagePluginIface : public PluginInterface
{
public:
    virtual ~PropertiesWindowTabPagePluginIface() {}

    virtual int tabOrder() = 0;
    virtual bool supportUris(const QStringList &uris) = 0;
//    virtual QWidget *createTabPage(const QStringList &uris) = 0;
    virtual PropertiesWindowTabIface *createTabPage(const QStringList &uris) = 0;

    virtual void closeFactory() = 0;

};

}

Q_DECLARE_INTERFACE(Peony::PropertiesWindowTabPagePluginIface, PropertiesWindowTabPagePluginIface_iid)

#endif // PROPERTIESWINDOWTABPAGEPLUGINIFACE_H
