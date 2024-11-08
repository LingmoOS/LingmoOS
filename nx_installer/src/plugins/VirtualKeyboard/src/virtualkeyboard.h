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
#ifndef VIRTUALKEYBOARD_H
#define VIRTUALKEYBOARD_H

#include <QWidget>
#include <QPushButton>

#include "keyboardwidget.h"
#include "cursormonitor.h"

class VirtualKeyboard : public QWidget
{
    Q_OBJECT
public:
    explicit VirtualKeyboard(QWidget *parent = 0);

private:
    void adjustGeometry(int screen);
    virtual void mouseMoveEvent(QMouseEvent*e);
    virtual void mousePressEvent(QMouseEvent* e);

signals:
    void aboutToClose();

private:
    KeyboardWidget      *keyboardWidget;
    CursorMonitor       *cursorMonitor;
    bool                isApplication;
    QPoint                 clickPos;
};


#endif // VIRTUALKEYBOARD_H
