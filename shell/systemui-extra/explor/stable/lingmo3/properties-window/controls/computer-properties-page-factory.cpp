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

#include "computer-properties-page-factory.h"

#include "computer-properties-page.h"

using namespace Peony;

static ComputerPropertiesPageFactory *global_instance = nullptr;

ComputerPropertiesPageFactory::ComputerPropertiesPageFactory(QObject *parent) : QObject(parent)
{

}

ComputerPropertiesPageFactory *ComputerPropertiesPageFactory::getInstance()
{
    if (!global_instance)
        global_instance = new ComputerPropertiesPageFactory;
    return global_instance;
}

bool ComputerPropertiesPageFactory::supportUris(const QStringList &uris)
{
    if (uris.count() != 1)
        return false;

    if (!uris.first().contains("computer:///"))
        return false;

    return true;
}

PropertiesWindowTabIface *ComputerPropertiesPageFactory::createTabPage(const QStringList &uris)
{
    return new ComputerPropertiesPage(uris.first());
}
