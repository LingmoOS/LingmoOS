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

#include "tablet-side-bar-factory.h"

#include "intel-navigation-side-bar.h"

using namespace Peony::Intel;
using namespace Intel;

TabletSideBarFactory::TabletSideBarFactory(QObject *parent) : QObject(parent)
{

}

const QStringList TabletSideBarFactory::keys()
{
    QStringList keys;
    keys.append("V10SP1-edu");
    keys.append("Intel");
    return keys;
}

Peony::SideBar *TabletSideBarFactory::create(QWidget *parent)
{
    return new NavigationSideBarContainer(parent);
}

Peony::PluginInterface::PluginType TabletSideBarFactory::pluginType()
{
    return SideBarPlugin;
}

const QString TabletSideBarFactory::name()
{
    return "Intel";
}

const QString TabletSideBarFactory::description()
{
    return tr("Intel Side Bar");
}

const QIcon TabletSideBarFactory::icon()
{
    return QIcon::fromTheme("file-manager");
}

void TabletSideBarFactory::setEnable(bool enable)
{

}

bool TabletSideBarFactory::isEnable()
{
    return true;
}
