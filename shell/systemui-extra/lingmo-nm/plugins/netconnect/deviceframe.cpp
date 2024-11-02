/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2022 Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "deviceframe.h"
#include <QPainterPath>

#define LAYOUT_MARGINS 16,0,16,0
#define FRAME_HEIGHT 58
#define RADIUS 6.0

DeviceFrame::DeviceFrame(QString devName, QWidget *parent) : QFrame(parent)
{
    this->setFrameShape(QFrame::Box);
    this->setFixedHeight(FRAME_HEIGHT);
    QHBoxLayout *deviceLayout = new QHBoxLayout(this);
    deviceLayout->setContentsMargins(LAYOUT_MARGINS);
    setLayout(deviceLayout);

    deviceLabel = new QLabel(this);
    dropDownLabel = new DrownLabel(devName, this);
//    deviceSwitch = new KSwitchButton(this);
//    deviceSwitch->installEventFilter(this);

    deviceLayout->addWidget(deviceLabel);
    deviceLayout->addStretch();
    deviceLayout->addWidget(dropDownLabel);/*
    deviceLayout->addWidget(deviceSwitch);*/
}

bool DeviceFrame::eventFilter(QObject *w,QEvent *e)
{
//    if (w == deviceSwitch) {
//        if (e->type() == QEvent::MouseButtonPress) {
//            emit deviceSwitchClicked(!deviceSwitch->isChecked());
//            return true;
//        }
//    }
    return QFrame::eventFilter(w, e);
}

void DeviceFrame::paintEvent(QPaintEvent *event)
{
    QPalette pal = this->palette();

    QPainter painter(this);
    painter.setRenderHint(QPainter:: Antialiasing, true);  //设置渲染,启动反锯齿
    painter.setPen(Qt::NoPen);
    painter.setBrush(pal.color(QPalette::Base));

    QRect rect = this->rect();
    QPainterPath path;
    path.addRoundedRect (rect, RADIUS, RADIUS);
    QRect temp_rect(rect.left(), rect.top() + rect.height()/2, rect.width(), rect.height()/2);
    path.addRect(temp_rect);
    painter.drawPath(path);
    QFrame::paintEvent(event);
}
