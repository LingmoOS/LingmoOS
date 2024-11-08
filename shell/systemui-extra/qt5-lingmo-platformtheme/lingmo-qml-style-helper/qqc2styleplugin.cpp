/*
 * Qt5-LINGMO's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 * Authors: Yan Wang <wangyan@kylinos.cn>
 *
 */

#include "qqc2styleplugin.h"
#include "kyquickstyleitem.h"
#include "kystylehelper.h"
#include "KyIcon.h"

void Qqc2StylePlugin::registerTypes(const char *uri)
{
    Q_ASSERT(QLatin1String(uri) == QLatin1String("org.lingmo.qqc2style.private"));
    // @uri org.lingmo.qqc2style.private
    qmlRegisterType<KyQuickStyleItem>(uri, 1, 0, "StyleItem");
    qmlRegisterType<KyStyleHelper>(uri, 1, 0, "StyleHelper");
    qmlRegisterType<KyIcon>(uri, 1, 0, "KyIcon");
}
