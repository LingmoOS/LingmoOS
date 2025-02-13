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

#ifndef APPITEM_H
#define APPITEM_H

#include <QString>
#include <QStringList>
#include <QMetaType>

class AppItem
{
public:
    AppItem();
    AppItem(const AppItem &info);
    ~AppItem();

    inline bool operator==(const AppItem &other) const { return id == other.id; }
    friend QDataStream &operator<<(QDataStream &argument, const AppItem &info);
    friend const QDataStream &operator>>(QDataStream &argument, AppItem &info);

    QString id;
    QString name;
    QString genericName;
    QString comment;
    QString iconName;
    QStringList args;

    bool newInstalled;
};

Q_DECLARE_METATYPE(AppItem)

#endif // APPITEM_H
