/*
 * Copyright (C) 2025 Lingmo OS Team.
 *
 * Author:     Lingmo OS Team <team@lingmo.org>
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
#include <QApplication>

#include "desktop/desktopview.h"

class Desktop : public QApplication
{
    Q_OBJECT

public:
    explicit Desktop(int& argc, char** argv);
    int run();

private slots:
    void screenAdded(QScreen *qscreen);
    void screenRemoved(QScreen *qscreen);

private:
    QMap<QScreen *, DesktopView *> m_list;
};//

#endif // DESKTOP_H
