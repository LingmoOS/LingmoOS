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
#ifndef WLANITEM_H
#define WLANITEM_H
#include <QWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QLabel>
#include <QVariantMap>
#include <QTimer>
#include <QDebug>
#include <QGSettings>
#include <QImage>
#include "fixlabel.h"
//#include "infobutton.h"
#include "../component/AddBtn/grayinfobutton.h"

class WlanItem : public QPushButton
{
public:
    WlanItem(bool bAcitve, bool isLock, QWidget *parent = nullptr);
    ~WlanItem();
public:
    QLabel * iconLabel = nullptr;
    GrayInfoButton * infoLabel = nullptr;
    FixLabel * titileLabel = nullptr;
    QLabel * statusLabel = nullptr;
    QString uuid = "";

    void setHalfFillet(bool flag) {useHalfFillet = flag; repaint();}
public:
    void startLoading();
    void stopLoading();
    bool isAcitve = false;
    bool loading = false;
    bool isLock = false;



protected:
    void paintEvent(QPaintEvent *event);

private:
    QTimer *waitTimer = nullptr;
    QGSettings *themeGsettings = nullptr;
    bool useHalfFillet = false;
    QList<QIcon> loadIcons;
    int currentIconIndex=0;

private slots:
    void updateIcon();

};

#endif // WLANITEM_H
