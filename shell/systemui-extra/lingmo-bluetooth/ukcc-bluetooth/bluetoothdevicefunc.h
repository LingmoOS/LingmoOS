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

#ifndef BLUETOOTHDEVICEFUNC_H
#define BLUETOOTHDEVICEFUNC_H

#include <QIcon>
#include <QMenu>
#include <QTimer>
#include <QDebug>
#include <QObject>
#include <QPalette>
#include <QDateTime>
#include <QMessageBox>
#include <QPushButton>
#include <QApplication>
#include <QGSettings/QGSettings>

#include "devicebase.h"
#include "devrenamedialog.h"
#include "devremovedialog.h"
//#include "bluetoothmainwindow.h"
#include "bluetoothdbusservice.h"

#include "windowmanager/windowmanager.h"

#define  ICON_PATH_NAME "view-more-horizontal-symbolic"

class bluetoothdevicefunc : public QPushButton
{
    Q_OBJECT

public:
    bluetoothdevicefunc(QWidget *parent ,QString dev_address);
    ~bluetoothdevicefunc();

    void initGsettings();
    void initInterface();
    void initBackground();
signals:
    void devFuncOpertionSignal();
    void devFuncOpertionRemoveSignal(QString);
    void devBtnReleaseSignal();
private slots:
    void MenuSignalDeviceFunction(QAction *action);
    void mStyle_GSettingsSlot(const QString &key);
    void MenuSignalAboutToHide();
protected:
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
private:


    QMenu   * devMenuFunc           = nullptr;
    QGSettings * _mStyle_GSettings  = nullptr;
    QString     _MDev_addr         ;
    bool        _themeIsBlack = false;
    QString     _mIconThemeName;

    long long _pressCurrentTime;

    void showDeviceRenameWidget();
    void showDeviceRemoveWidget(DevRemoveDialog::REMOVE_INTERFACE_TYPE mode);
};


#endif // BLUETOOTHDEVICEFUNC_H
