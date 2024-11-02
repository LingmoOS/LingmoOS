/**
 * Copyright (C) 2018 Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301, USA.
**/
#ifndef CURSORMONITOR_H
#define CURSORMONITOR_H

#include <QObject>
#include <QThread>

class CursorMonitor : public QThread
{
    Q_OBJECT
public:
    explicit CursorMonitor(QObject *parent = nullptr);
    void run();

Q_SIGNALS:
    void cursorPosChanged(const QPoint& pos);
};

#endif // CURSORMONITOR_H
