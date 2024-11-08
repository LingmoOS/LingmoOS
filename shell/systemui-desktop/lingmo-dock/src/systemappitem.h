/*
 * Copyright (C) 2021 LingmoOS Team.
 *
 * Author:     rekols <revenmartin@gmail.com>
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

#ifndef SYSTEMAPPITEM_H
#define SYSTEMAPPITEM_H

#include <QObject>

class SystemAppItem : public QObject
{
    Q_OBJECT

public:
    explicit SystemAppItem(QObject *parent = nullptr);

    QString path;
    QString name;
    QString genericName;
    QString comment;
    QString iconName;
    QString startupWMClass;
    QString exec;
    QStringList args;
};

#endif // SYSTEMAPPITEM_H
