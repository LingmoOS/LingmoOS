/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, Tianjin LINGMO Information Technology Co., Ltd.
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

#ifndef STABLEPROPERTIESWINDOWFACTORY_H
#define STABLEPROPERTIESWINDOWFACTORY_H

#include <QObject>
#include <QMainWindow>

#include "properties-window-factory-plugin-iface.h"
#include "properties-window_global.h"

namespace Peony {

class PropertiesWindowTabPagePluginIface;

/*!
 * \brief The StablePropertiesWindowFactory class
 * \deprecated
 */
class STABLEPROPERTIESWINDOW_EXPORT StablePropertiesWindowFactory : public QObject, public PropertiesWindowFactoryPluginIface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID PropertiesWindowFactoryPluginIface_iid FILE "common.json")
    Q_INTERFACES(Peony::PropertiesWindowFactoryPluginIface)

public:
    static StablePropertiesWindowFactory *getInstance();

    //plugin iface
    const QString version() override {
        return "lingmo3.0";
    }
    const QString name() override {
        return "lingmo3.0";
    }
//    PluginType pluginType() override {
//        return PluginType::PropertiesWindowPlugin;
//    }
    const QString description() override {
        return QObject::tr("Show properties plugin window.");
    }
    const QIcon icon() override {
        return QIcon::fromTheme("view-paged-symbolic", QIcon::fromTheme("folder"));
    }
    void setEnable(bool enable) override {
        Q_UNUSED(enable)
    }
    bool isEnable() override {
        return true;
    }

    QMainWindow *create(const QStringList &uris);
    void closeFactory();

    explicit StablePropertiesWindowFactory(QObject *parent = nullptr);
    ~StablePropertiesWindowFactory() override {}

    bool registerFactory(QObject *factory);
    bool unregisterFactory(QObject *factory);
//    Peony::PropertiesWindowPluginManager *getManager() {
//        return m_manager;
//    }
//private:
//    Peony::PropertiesWindowPluginManager *m_manager = nullptr;
};

}

#endif // STABLEPROPERTIESWINDOWFACTORY_H
