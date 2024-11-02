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
#ifndef DEVICEFRAME_H
#define DEVICEFRAME_H
#include <QObject>
#include <QWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QLabel>
#include <QTimer>
#include <QDebug>
#include "../component/DrownLabel/drownlabel.h"
#include "kwidget.h"
#include "kswitchbutton.h"

using namespace kdk;

class DeviceFrame : public QFrame
{

    Q_OBJECT
public:
    DeviceFrame(QString devName, QWidget *parent = nullptr);
public:
    QLabel * deviceLabel = nullptr;
    KSwitchButton * deviceSwitch = nullptr;
    DrownLabel *dropDownLabel = nullptr;

protected:
    void paintEvent(QPaintEvent *event);
    bool eventFilter(QObject *w,QEvent *e);

private:
    bool isDropDown = false;
    int frameSize;

signals:
    void deviceSwitchClicked(bool);
};

#endif // DEVICEFRAME_H
