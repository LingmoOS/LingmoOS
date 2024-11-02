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
 * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */

#ifndef LINGMO_SEARCH_ICON_LOADER_H
#define LINGMO_SEARCH_ICON_LOADER_H
#include <QIcon>
namespace LingmoUISearch {

class IconLoader
{
public:
    static QIcon loadIconQt(const QString &name, const QIcon &fallback = {});
    static QIcon loadIconXdg(const QString &name, const QIcon &fallback = {});
};

} // LingmoUISearch

#endif //LINGMO_SEARCH_ICON_LOADER_H
