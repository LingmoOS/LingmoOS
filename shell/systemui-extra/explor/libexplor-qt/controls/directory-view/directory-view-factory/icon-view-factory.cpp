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

#include "icon-view-factory.h"
#include "icon-view.h"

using namespace Peony;

static IconViewFactory *globalInstance = nullptr;
static IconViewFactory2 *globalInstance2 = nullptr;

IconViewFactory *IconViewFactory::getInstance()
{
    if (!globalInstance) {
        globalInstance = new IconViewFactory;
    }
    return globalInstance;
}

IconViewFactory::IconViewFactory(QObject *parent) : QObject (parent)
{

}

IconViewFactory::~IconViewFactory()
{

}

DirectoryViewIface *IconViewFactory::create()
{
    return new Peony::DirectoryView::IconView;
}

//Factory2
IconViewFactory2 *IconViewFactory2::getInstance()
{
    if (!globalInstance2) {
        globalInstance2 = new IconViewFactory2;
    }
    return globalInstance2;
}

IconViewFactory2::IconViewFactory2(QObject *parent) : QObject (parent)
{

}

IconViewFactory2::~IconViewFactory2()
{

}

DirectoryViewWidget *IconViewFactory2::create()
{
    return new Peony::DirectoryView::IconView2;
}
