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

#ifndef DESKTOPVIEW_H
#define DESKTOPVIEW_H

#include <QQuickView>
#include <QScreen>

class Desktop;
class DesktopView : public QQuickView
{
    Q_OBJECT
    Q_PROPERTY(QRect screenRect READ screenRect NOTIFY screenRectChanged)

public:
    explicit DesktopView(QScreen *screen = nullptr, QQuickView *parent = nullptr);

    QRect screenRect();

signals:
    void screenRectChanged();

private slots:
    void onPrimaryScreenChanged(QScreen *screen);
    void onGeometryChanged();

private:
    QScreen *m_screen;
    QRect m_screenRect;
};

#endif // DESKTOPVIEW_H
