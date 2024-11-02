/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
**/

#ifndef LOADINGLABEL_H
#define LOADINGLABEL_H

#include <QIcon>
#include <QDebug>
#include <QTimer>
#include <QWidget>
#include <QPainter>
#include <QGSettings>
#include <QPaintEvent>

#include "config.h"
#include "ukccbluetoothconfig.h"

class LoadingLabel : public QWidget
{
    Q_OBJECT
public:
    explicit LoadingLabel(QObject *parent = nullptr);
    ~LoadingLabel();
    void setTimerStop();
    void setTimerStart();
    void setTimeReresh(int);

private Q_SLOTS:
    void refreshIconNum();
    void mStyle_GSettingsSlot(const QString &key);

protected:
    void paintEvent(QPaintEvent *e);

private:
    int          index = 0;
    QTimer     * m_timer;
    bool         _themeIsBlack = false;
    QGSettings * _mStyle_GSettings  = nullptr;

private:
    void initGsettings();
};

#endif // LOADINGLABEL_H
