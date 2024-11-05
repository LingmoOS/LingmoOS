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

#include "permissions-properties-page-factory.h"
#include "permissions-properties-page.h"

#include <QLabel>

using namespace Peony;

static PermissionsPropertiesPageFactory *global_instance = nullptr;

PermissionsPropertiesPageFactory::PermissionsPropertiesPageFactory(QObject *parent) : QObject(parent)
{

}

bool PermissionsPropertiesPageFactory::supportUris(const QStringList &uris)
{
    //FIXME: 需要明确支持范围
    //FIXME: Need to clarify the scope of support
    if (uris.count() != 1)
        return false;

    QString uri = uris.first();

    if (uri.startsWith("computer://") || uri.startsWith("recent://") || uri.startsWith("trash://"))
        return false;

    return true;
}

PropertiesWindowTabIface *PermissionsPropertiesPageFactory::createTabPage(const QStringList &uris)
{
    return new PermissionsPropertiesPage(uris);
}

void PermissionsPropertiesPageFactory::closeFactory()
{
    this->deleteLater();
}

PermissionsPropertiesPageFactory *PermissionsPropertiesPageFactory::getInstance()
{
    if (!global_instance)
        global_instance = new PermissionsPropertiesPageFactory;
    return global_instance;
}
