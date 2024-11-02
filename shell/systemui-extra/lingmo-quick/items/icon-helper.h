/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: hxf <hewenfei@kylinos.cn>
 *
 */

#ifndef LINGMO_QUICK_ITEMS_ICON_HELPER_H
#define LINGMO_QUICK_ITEMS_ICON_HELPER_H

#include <QUrl>
#include <QString>
#include <QIcon>
#include <QPixmap>

namespace LingmoUIQuick {

class IconHelper
{
public:
    // 判断函数
    // 是否存在主题图标
    static bool isThemeIcon(const QString &name);
    // 是否本地文件
    static bool isLocalFile(const QUrl &url);
    // 远程文件： http or https
    static bool isRemoteServerFile(const QUrl &url);

    // 功能函数
    /**
     * Url转换为本地文件
     * 判断一个url是否本地文件。包括qrc
     * @param url
     * @return 返回可用于load的path，如果不是本地文件，返回空
     */
    static QString toLocalPath(const QUrl &url);

    // 图标相关
    // 从某个路径加载图标，并存入引用中，返回加载是否成功
    static bool loadPixmap(const QString &path, QPixmap &pixmap);

    // 从路径或者主题加载图标
    static QIcon loadIcon(const QString &id);

    // 默认图标
    static QIcon getDefaultIcon();

private:
    static bool loadThemeIcon(const QString &name, QIcon &icon);
    static bool loadXdgIcon(const QString &name, QIcon &icon);
    static void loadDefaultIcon(QIcon &icon);
};

} // LingmoUIQuick

#endif //LINGMO_QUICK_ITEMS_ICON_HELPER_H
