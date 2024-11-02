/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2023, Tianjin LINGMO Information Technology Co., Ltd.
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
 * Authors: Yue Lan <yangyanwei@kylinos.cn>
 *
 */

#include "properties-window-factory.h"
#include "properties-window.h"

using namespace Peony;

static StablePropertiesWindowFactory *global_instance = nullptr;

StablePropertiesWindowFactory::StablePropertiesWindowFactory(QObject *parent) : QObject(parent)
{
    PropertiesWindowPluginManager::getInstance()->initFactory();
}

StablePropertiesWindowFactory *StablePropertiesWindowFactory::getInstance()
{
    if (!global_instance)
        global_instance = new StablePropertiesWindowFactory;
    return global_instance;
}

QMainWindow *StablePropertiesWindowFactory::create(const QStringList &uris)
{
    auto window = new StablePropertiesWindow(uris);
    return window;
}

void StablePropertiesWindowFactory::closeFactory()
{
    this->deleteLater();
}

bool StablePropertiesWindowFactory::registerFactory(QObject *factory)
{
    PropertiesWindowTabPagePluginIface *Iface = dynamic_cast<PropertiesWindowTabPagePluginIface*>(factory);
    return PropertiesWindowPluginManager::getInstance()->registerFactory(Iface);
}

bool StablePropertiesWindowFactory::unregisterFactory(QObject *factory)
{
    PropertiesWindowTabPagePluginIface *Iface = dynamic_cast<PropertiesWindowTabPagePluginIface*>(factory);
    return PropertiesWindowPluginManager::getInstance()->unregisterFactory(Iface);
}
