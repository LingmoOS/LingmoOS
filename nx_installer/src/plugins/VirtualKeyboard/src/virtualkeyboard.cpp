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
#include "virtualkeyboard.h"
#include <QFile>
#include <QDesktopWidget>
#include <QHBoxLayout>
#include <QApplication>
#include <QMouseEvent>
#include <QDebug>

VirtualKeyboard::VirtualKeyboard(QWidget *parent)
    : QWidget(parent)
{
    Q_INIT_RESOURCE(keyboard);

//    QPixmap pixmap = QPixmap(":/data/png/bg_message.jpg");
    QPalette pal(this->palette());
//    pal.setBrush(backgroundRole(), QBrush(pixmap));
    pal.setBrush(backgroundRole(), Qt::black);
    this->setAutoFillBackground(true);
    this->setPalette(pal);

    setWindowFlags(Qt::FramelessWindowHint |
                   Qt::WindowStaysOnTopHint |
                   Qt::WindowDoesNotAcceptFocus);

    keyboardWidget = new KeyboardWidget(this);
    QHBoxLayout *hl_keyboard = new QHBoxLayout(this);
    QSpacerItem *spacer = new QSpacerItem(20, 20);
    hl_keyboard->addSpacerItem(spacer);
    hl_keyboard->addWidget(keyboardWidget);
    QSpacerItem *spacer2 = new QSpacerItem(20, 20);
    hl_keyboard->addSpacerItem(spacer2);

    QFile qssFile(":/qss/keyboard.qss");
    qDebug() << qssFile.exists();
    qssFile.open(QIODevice::ReadOnly);
    setStyleSheet(qssFile.readAll());
    qssFile.close();

    QDesktopWidget *desktop = QApplication::desktop();
    cursorMonitor = new CursorMonitor(this);

    //在多显示器情况下，监视鼠标指针的位置和主显示器变化信号
    connect(cursorMonitor, &CursorMonitor::cursorPosChanged,
            this, [&](const QPoint &pos){
        adjustGeometry(desktop->screenNumber(pos));
    });

    connect(desktop, &QDesktopWidget::primaryScreenChanged,
            this, [&]{
        adjustGeometry(desktop->primaryScreen());
    });

    connect(keyboardWidget, &KeyboardWidget::aboutToClose,
            this, &VirtualKeyboard::aboutToClose);

    adjustGeometry(desktop->primaryScreen());

}

void VirtualKeyboard::adjustGeometry(int screen)
{
//    QDesktopWidget *desktop = QApplication::desktop();
//    QWidget *activateScreen = desktop->screen(screen);
//    setGeometry(activateScreen->width() / 3, activateScreen->height() - activateScreen->height() * 3 / 4,
//                activateScreen->width() / 2, activateScreen->height() / 3);
    QRect rect =  QApplication::desktop()->rect();
//    setGeometry(0, 0, 520, 250);
    setGeometry(rect.width() - 780, rect.height() - 250, 780, 250);
}

void VirtualKeyboard::mousePressEvent(QMouseEvent *e)
{
    if(e->button() == Qt::LeftButton)
        clickPos = e->pos();
}


void VirtualKeyboard::mouseMoveEvent(QMouseEvent *e)
{
    if(e->buttons()&Qt::LeftButton)
        move(e->pos() + pos() - clickPos);
}
