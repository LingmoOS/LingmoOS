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

#ifndef DOCKDBUSINTERFACE_H
#define DOCKDBUSINTERFACE_H

#include <QObject>
#include <QDBusInterface>

class DockDBusInterface : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int leftMargin READ leftMargin NOTIFY marginsChanged)
    Q_PROPERTY(int rightMargin READ rightMargin NOTIFY marginsChanged)
    Q_PROPERTY(int bottomMargin READ bottomMargin NOTIFY marginsChanged)

public:
    static DockDBusInterface *self();
    explicit DockDBusInterface(QObject *parent = nullptr);

    int leftMargin() const;
    int rightMargin() const;
    int bottomMargin() const;

signals:
    void marginsChanged();

private slots:
    void updateMargins();

private:
    QDBusInterface m_dockInterface;

    int m_leftMargin;
    int m_rightMargin;
    int m_bottomMargin;
};

#endif // DOCKDBUSINTERFACE_H
