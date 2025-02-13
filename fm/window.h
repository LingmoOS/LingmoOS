/*
 * Copyright (C) 2021 LingmoOS Team.
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

#ifndef WINDOW_H
#define WINDOW_H

#include <QQmlApplicationEngine>

class Window : public QQmlApplicationEngine
{
    Q_OBJECT

public:
    explicit Window(QObject *parent = nullptr);

    void load(const QUrl &url);

protected:
    bool eventFilter(QObject *o, QEvent *e);
};

#endif // WINDOW_H
