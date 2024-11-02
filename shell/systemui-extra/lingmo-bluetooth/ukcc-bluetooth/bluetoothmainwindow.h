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

#ifndef BLUETOOTHMAINWINDOW_H
#define BLUETOOTHMAINWINDOW_H

#include <QIcon>
#include <QTimer>
#include <QLabel>
#include <QFrame>
#include <QDebug>
#include <QObject>
#include <QString>
#include <QWidget>
#include <QComboBox>
#include <QMetaEnum>
#include <QMainWindow>
#include <QStringList>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStackedWidget>

#include "loadinglabel.h"
#include "kswitchbutton.h"
#include "bluetoothnamelabel.h"

#include "bluetoothdeviceitem.h"
#include "bluetoothdevicewindowitem.h"
#include "bluetoothmainnormalwindow.h"
#include "bluetoothmainerrorwindow.h"
#include "bluetoothmainloadingwindow.h"

enum Window_Type
{
    MAINWINDOW_ERROR_WINDOW = 0,
    MAINWINDOW_LOADING_INTERFACE,
    MAINWINDOW_NORMAL_INTERFACE,

    WINDOW_COUNT
};


using namespace kdk;
class BluetoothMainNormalWindow;

class BlueToothMainWindow : public QMainWindow
{
    Q_OBJECT
public:
    //记录当前是否蓝牙界面是否在操作状态
    static bool m_device_operating; //有设备正在操作
    explicit BlueToothMainWindow(QWidget *parent = nullptr);
    ~BlueToothMainWindow();
private slots:
    void loadingTimeOutSlot();

    //dev slot
    void adapterAddSlot(QString adapter_name);
    void adapterRemoveSlot(int indx);

    void btServiceRestartSlot();
    void btServiceRestartCompleteSlot(bool);
    void defaultAdapterChangedSlot(int indx);

private:
    QStackedWidget              * m_centralWidget   = nullptr;
    BluetoothMainNormalWindow   * m_normalWindow    = nullptr;
    BluetoothMainErrorWindow    * m_errorWindow     = nullptr;
    BluetoothMainLoadingWindow  * m_loadingWindow   = nullptr;

    QTimer * m_loadingTimer = nullptr;

    BlueToothDBusService * btServer = nullptr;
    int m_errType_id ;
    int m_init_btServer_res ;
    int m_btServer_errId ;

private:
    void InitBTServerConnection();
    void InitWindows();
    void InitNormalWindow();
    void InitErrorWindow();
    void InitLoadingWindow();
    void InitLoadingTimer();


    void displayNormalWindow();
    void displayErrorAbnormalWindow();
    void displayErrorNoAdapterWindow();
    void displayErrorBluetoothServerInitFailedWindow();
    void displayErrorUnknownWindow();
    void displayLoadingWindow();
};

#endif // BLUETOOTHMAINWINDOW_H
