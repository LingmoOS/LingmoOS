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

#ifndef TRAYICON_H
#define TRAYICON_H

#include <glib.h>
#include <glib/gprintf.h>
#include <iostream>
#include <QApplication>
#include <QWidget>
#include <QSystemTrayIcon>
#include <QAction>
#include <QString>
#include <QMenu>
#include <QGSettings>
#include <QDebug>

#ifdef KYSEC
#include <kysec/libkysec.h>
#include <kysec/status.h>
#endif

#include "config/config.h"

using namespace std;

class TrayIcon : public QWidget
{
    Q_OBJECT

public:
    TrayIcon(bool _trayshow, bool _existAdapter, QWidget *parent = nullptr);
    ~TrayIcon();
    void SetAdapterFlag(bool set);
    void setClickEnable(bool enable);
    void setAdapterExist(bool exist, bool show);
    void updateTrayiconMenuStyle();

signals:
    void showTrayWidget();
    void hideTrayWidget();
    void openBluetoothSettings();

public slots:
    void SetTrayIcon(bool open);

private:
    QSystemTrayIcon *bluetooth_tray_icon = nullptr;
    QMenu           *tray_Menu           = nullptr;
    QGSettings      *settings            = nullptr;
    QGSettings      *ukccGsetting        = nullptr;
    bool            existAdapter         = false;
    bool            secState             = false;
    bool            clickEnable          = true;
    bool            existSettings        = false;
    bool            canShow              = false;
} ;
#endif // TRAYICON_H
