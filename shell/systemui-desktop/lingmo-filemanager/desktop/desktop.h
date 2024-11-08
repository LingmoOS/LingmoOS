/*
 * Copyright (C) 2021 LingmoOS Team.
 *
 * Author:     Reion Wong <reionwong@gmail.com>
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

#ifndef DESKTOP_H
#define DESKTOP_H

#include <QObject>
#include <QScreen>
#include <QDBusInterface>

#include "desktopview.h"

class Desktop : public QObject
{
    Q_OBJECT

public:
    explicit Desktop(QObject *parent = nullptr);

private slots:
    void screenAdded(QScreen *qscreen);
    void screenRemoved(QScreen *qscreen);

private:
    QMap<QScreen *, DesktopView *> m_list;
};

#endif // DESKTOP_H
