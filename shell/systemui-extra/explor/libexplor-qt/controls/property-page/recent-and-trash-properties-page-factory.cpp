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

#include "recent-and-trash-properties-page-factory.h"
#include "recent-and-trash-properties-page.h"

#include <QWidget>

using namespace Peony;

static RecentAndTrashPropertiesPageFactory *global_instance = nullptr;

RecentAndTrashPropertiesPageFactory *RecentAndTrashPropertiesPageFactory::getInstance()
{
    if (!global_instance)
        global_instance = new RecentAndTrashPropertiesPageFactory;
    return global_instance;
}

RecentAndTrashPropertiesPageFactory::RecentAndTrashPropertiesPageFactory(QObject *parent) : QObject(parent)
{

}

bool RecentAndTrashPropertiesPageFactory::supportUris(const QStringList &uris)
{
    if (uris.count() != 1)
        return false;

    if (uris.first().contains("trash:///") || uris.first().contains("recent:///"))
        return true;

    return false;
}

PropertiesWindowTabIface *RecentAndTrashPropertiesPageFactory::createTabPage(const QStringList &uris)
{
    return new RecentAndTrashPropertiesPage(uris);
}
