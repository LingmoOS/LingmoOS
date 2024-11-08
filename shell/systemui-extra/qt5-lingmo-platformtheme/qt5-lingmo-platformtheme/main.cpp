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
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include <qpa/qplatformthemeplugin.h>
#include "qt5-lingmo-platform-theme.h"

#include <QDebug>
#include <syslog.h>

#define LINGMO_PLATFORMTHEME

QT_BEGIN_NAMESPACE

class Qt5LINGMOPlatformThemePlugin : public QPlatformThemePlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QPlatformThemeFactoryInterface_iid FILE "lingmo.json")

public:
    virtual QPlatformTheme *create(const QString &key, const QStringList &params) {
        if (key.toLower() == "lingmo") {
            return new Qt5LINGMOPlatformTheme(params);
        }
        return nullptr;
    }
};

QT_END_NAMESPACE

#include "main.moc"
