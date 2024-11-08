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

#ifndef PROPERTIESWINDOWFACTORYPLUGINIFACE_H
#define PROPERTIESWINDOWFACTORYPLUGINIFACE_H

#include <QPluginLoader>
#include <QtPlugin>
#include <QStringList>
#include <QMainWindow>
#include "properties-window-tab-page-plugin-iface.h"

#define PropertiesWindowFactoryPluginIface_iid "org.lingmo.explorer-qt.plugin-iface.PropertiesWindowFactoryPluginIface"

namespace Peony {
class PropertiesWindowFactoryPluginIface
{
public:
    virtual const QString name() = 0;
    virtual const QString description() = 0;
    virtual const QIcon icon() = 0;
    virtual void setEnable(bool enable) = 0;
    virtual bool isEnable() = 0;
    virtual ~PropertiesWindowFactoryPluginIface() {}

    virtual const QString version() = 0;
    virtual QMainWindow *create(const QStringList &uris) = 0;
    virtual void closeFactory() = 0;
    virtual bool registerFactory(QObject *factory) = 0;
    virtual bool unregisterFactory(QObject *factory) = 0;
};
}

Q_DECLARE_INTERFACE(Peony::PropertiesWindowFactoryPluginIface, PropertiesWindowFactoryPluginIface_iid)

#endif // PROPERTIESWINDOWTABPAGEPLUGINIFACE_H
