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

#ifndef BLUETOOTHMIDDLEWINDOW_H
#define BLUETOOTHMIDDLEWINDOW_H

#include <QList>
#include <QLabel>
#include <QObject>
#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>

#include "bluetoothdbusservice.h"
#include "bluetoothdevicewindowitem.h"

class BluetoothMiddleWindow : public QWidget
{
    Q_OBJECT
public:
    BluetoothMiddleWindow(BlueToothDBusService * btServer,QWidget * parent = nullptr);
    ~BluetoothMiddleWindow();

    void reloadWindow();
    void quitWindow();
Q_SIGNALS:
    void myDeviceWindosHiddenSignal(bool status);

private Q_SLOTS:
    void deviceRemoveSlot(QString dev_address);
    void devicePairedSuccessSlot(QString dev_address);
    void defaultAdapterChangedSlot(int indx);
    void devConnectedChangedSlot(QString dev_address,bool status);
private:

    BlueToothDBusService * m_btServer = nullptr;

    QFrame  *_MNormalFrameMiddle = nullptr;
    QVBoxLayout *_NormalWidgetPairedDevLayout = nullptr;
    QWidget  *_MNotConnectedNormalFrameMiddle = nullptr;
    QVBoxLayout *_NormalWidgetNotConnecededDevLayout = nullptr;
    QWidget  *_MConnectedNormalFrameMiddle = nullptr;
    QVBoxLayout *_NormalWidgetConnecededDevLayout = nullptr;

    //bool ;
private:

    void InitNormalWidgetMiddle();
    void InitConnectionData();
//    void AddMyDeviceWidgetConnectedAndNotConnected();
    void AddMyBluetoothDevices();
    void addMyDeviceItemUI(QString device_address);
    void removeMyDeviceItemUI(QString device_address);
    void setLastDevItemWindowLine(bool status);
    void clearMyDevicesUI();
//    int getConnectedCount();
//    int getNotConnectedCount();
};

#endif // BLUETOOTHMIDDLEWINDOW_H
