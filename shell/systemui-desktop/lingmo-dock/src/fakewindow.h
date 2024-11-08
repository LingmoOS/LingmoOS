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

#ifndef FAKEWINDOW_H
#define FAKEWINDOW_H

#include <QQuickView>
#include <QTimer>

class FakeWindow : public QQuickView
{
    Q_OBJECT

public:
    explicit FakeWindow(QQuickView *parent = nullptr);

    bool containsMouse() const;
    void updateGeometry();

signals:
    void containsMouseChanged(bool contains);
    void dragEntered();

protected:
    bool event(QEvent *e) override;

private:
    void setContainsMouse(bool contains);

private:
    QTimer m_delayedMouseTimer;

    bool m_delayedContainsMouse;
    bool m_containsMouse;
};

#endif // FAKEWINDOW_H
