/*
 * Copyright (C) 2021 LingmoOS.
 *
 * Author:     Reion Wong <reion@lingmoos.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "appitem.h"

AppItem::AppItem()
    : newInstalled(false)
{

}

AppItem::AppItem(const AppItem &info)
    : id(info.id)
    , name(info.name)
    , genericName(info.genericName)
    , comment(info.comment)
    , iconName(info.iconName)
    , args(info.args)
    , newInstalled(false)
{

}

AppItem::~AppItem()
{

}

QDataStream &operator<<(QDataStream &argument, const AppItem &info)
{
    argument << info.id << info.name << info.genericName;
    argument << info.comment << info.iconName;

    return argument;
}

const QDataStream &operator>>(QDataStream &argument, AppItem &info)
{
    argument >> info.id >> info.name >> info.genericName;
    argument >> info.comment >> info.iconName;

    return argument;
}
