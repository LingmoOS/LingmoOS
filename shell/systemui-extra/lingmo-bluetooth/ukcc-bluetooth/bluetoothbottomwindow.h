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
#ifndef BLUETOOTHBOTTOMWINDOW_H
#define BLUETOOTHBOTTOMWINDOW_H


#include <QLabel>
#include <QObject>
#include <QWidget>
#include <QComboBox>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include "loadinglabel.h"
#include "bluetoothdbusservice.h"
#include "bluetoothdevicewindowitem.h"

class BluetoothBottomWindow : public QWidget
{
    Q_OBJECT
public:
    enum _DEV_TYPE {
        BT_All = 0,
        BT_Audio,
        BT_Peripherals,
        BT_Computer,
        BT_Phone,
        BT_Other,
    };
    Q_ENUM(_DEV_TYPE);

    BluetoothBottomWindow(BlueToothDBusService * btServer,QWidget * parent = nullptr);
    ~BluetoothBottomWindow();

    const QStringList devTypeSelectStrList = {tr("All"), \
                                              tr("Audio"), \
                                              tr("Peripherals"), \
                                              tr("Computer"), \
                                              tr("Phone"), \
                                              tr("Other")};

    void reloadWindow();
    void quitWindow();

private Q_SLOTS:

    void adapterDiscoveringSlot(bool status);
    void _DevTypeSelectComboBoxSlot(int indx);
    void deviceAddSlot(QString dev_address);
    void deviceRemoveSlot(QString dev_address);
    void devicePairedSuccessSlot(QString dev_address);
    void adjustDevItemDisplayPosition(QString address,quint16 rssiValue);
    void defaultAdapterChangedSlot(int);
private:

    BlueToothDBusService    * m_btServer = nullptr;

    QFrame                  * _MNormalFrameBottom = nullptr;
    LoadingLabel            * _LoadIcon = nullptr;
    QComboBox               * _DevTypeSelectComboBox = nullptr;
    QVBoxLayout             * _NormalWidgetCacheDevLayout = nullptr;

    _DEV_TYPE                 currentShowTypeFlag = _DEV_TYPE::BT_All;

private:
    void InitNormalWidgetBottom();
    void InitConnectionData();
    void AddBluetoothDevices();
    void addOneBluetoothDeviceItemUi(QString dev_address);
    void reloadDeviceListItem(BluetoothBottomWindow::_DEV_TYPE flag);
    void removeBluetoothDeviceItemUi(QString address);
    void setLastDevItemWindowLine(bool status);
    void clearOtherDevicesUI();

    bool whetherToDisplayInTheCurrentInterface(bluetoothdevice::DEVICE_TYPE devType);
    int  getDevRssiItemInsertIndex(qint16 currDevRssi);

};

#endif // BLUETOOTHBOTTOMWINDOW_H
