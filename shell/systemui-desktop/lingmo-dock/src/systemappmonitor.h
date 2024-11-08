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

#ifndef SYSTEMAPPMONITOR_H
#define SYSTEMAPPMONITOR_H

#include <QObject>
#include "systemappitem.h"

class SystemAppMonitor : public QObject
{
    Q_OBJECT

public:
    static SystemAppMonitor *self();

    explicit SystemAppMonitor(QObject *parent = nullptr);
    ~SystemAppMonitor();

    SystemAppItem *find(const QString &filePath);
    QList<SystemAppItem *> applications() { return m_items; }

signals:
    void refreshed();

private:
    void refresh();
    void addApplication(const QString &filePath);
    void removeApplication(SystemAppItem *item);

private:
    QList<SystemAppItem *> m_items;
};

#endif // SYSTEMAPPMONITOR_H
