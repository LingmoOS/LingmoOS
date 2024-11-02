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

#include "list-view-factory.h"
#include "list-view.h"

using namespace Peony;

static ListViewFactory *globalInstance = nullptr;
static ListViewFactory2 *globalInstance2 = nullptr;

ListViewFactory *ListViewFactory::getInstance()
{
    if (!globalInstance) {
        globalInstance = new ListViewFactory;
    }
    return globalInstance;
}

ListViewFactory::ListViewFactory(QObject *parent) : QObject (parent)
{

}

ListViewFactory::~ListViewFactory()
{

}

DirectoryViewIface *ListViewFactory::create()
{
    return new Peony::DirectoryView::ListView;
}

//List View 2
ListViewFactory2 *ListViewFactory2::getInstance()
{
    if (!globalInstance2) {
        globalInstance2 = new ListViewFactory2;
    }
    return globalInstance2;
}

ListViewFactory2::ListViewFactory2(QObject *parent) : QObject (parent)
{

}

ListViewFactory2::~ListViewFactory2()
{

}

DirectoryViewWidget *ListViewFactory2::create()
{
    return new Peony::DirectoryView::ListView2;
}
