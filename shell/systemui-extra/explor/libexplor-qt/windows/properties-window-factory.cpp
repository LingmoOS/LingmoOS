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
 * Authors: Yan Wei Yang <yangyanwei@kylinos.cn>
 *
 */

#include "properties-window-factory.h"
#include "properties-window.h"

using namespace Peony;

PropertiesWindowFactory *globalInstance = nullptr;

PropertiesWindowFactory::PropertiesWindowFactory(QObject *parent) : QObject(parent)
{

}

PropertiesWindowFactory *PropertiesWindowFactory::getInstance()
{
    if (!globalInstance)
        globalInstance = new PropertiesWindowFactory;
    return globalInstance;
}

QMainWindow *PropertiesWindowFactory::create(const QStringList &uris)
{
    auto window = new PropertiesWindow(uris);
    return window;
}

void PropertiesWindowFactory::closeFactory()
{
    this->deleteLater();
}

bool PropertiesWindowFactory::registerFactory(QObject *factory)
{
    PropertiesWindowTabPagePluginIface *Iface = dynamic_cast<PropertiesWindowTabPagePluginIface*>(factory);
    return PropertiesWindowPluginManager::getInstance()->registerFactory(Iface);
}

bool PropertiesWindowFactory::unregisterFactory(QObject *factory)
{
    PropertiesWindowTabPagePluginIface *Iface = dynamic_cast<PropertiesWindowTabPagePluginIface*>(factory);
    return PropertiesWindowPluginManager::getInstance()->unregisterFactory(Iface);
}
