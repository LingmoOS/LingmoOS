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

#include "default-preview-page-factory.h"
#include "default-preview-page.h"

using namespace Peony;

static DefaultPreviewPageFactory *globalInstance = nullptr;

DefaultPreviewPageFactory *DefaultPreviewPageFactory::getInstance()
{
    if (!globalInstance) {
        globalInstance = new DefaultPreviewPageFactory;
    }
    return globalInstance;
}

DefaultPreviewPageFactory::DefaultPreviewPageFactory(QObject *parent) : QObject(parent)
{

}

DefaultPreviewPageFactory::~DefaultPreviewPageFactory()
{

}

PreviewPageIface *DefaultPreviewPageFactory::createPreviewPage()
{
    return new DefaultPreviewPage;
}
