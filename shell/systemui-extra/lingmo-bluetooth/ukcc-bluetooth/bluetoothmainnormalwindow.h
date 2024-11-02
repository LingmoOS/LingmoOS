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
#ifndef BLUETOOTHMAINNORMALWINDOW_H
#define BLUETOOTHMAINNORMALWINDOW_H

#include <QTimer>
#include <QWidget>
#include <QObject>

#include "bluetoothdbusservice.h"
#include "bluetoothtopwindow.h"
#include "bluetoothmiddlewindow.h"
#include "bluetoothbottomwindow.h"

class BluetoothMiddleWindow;
class BluetoothBottomWindow;

class BluetoothMainNormalWindow : public QWidget
{
    Q_OBJECT

public:
    BluetoothMainNormalWindow(BlueToothDBusService * btServer,QWidget * parent = nullptr);
    ~BluetoothMainNormalWindow();

    void SetHidden(bool);
    void reloadWindow();
    void quitWindow();
private Q_SLOTS:
    void BluetoothSwitchChanged(bool);
    void defaultAdapterChangedSlot(int);
    void setHiddenForMyDevice(bool status);
private:
    BluetoothTopWindow      * m_topWidget = nullptr;
    BluetoothMiddleWindow   * m_middleWidget = nullptr;
    BluetoothBottomWindow   * m_bottomWidget = nullptr;

    QVBoxLayout *_NormalWidgetMainLayout = nullptr;
    BlueToothDBusService    * m_btServer = nullptr;

    bool m_defaultPowerStatus = true;

private:
    void Init();
    void InitConnectData();
    void InitDisplayStatus();


};

#endif // BLUETOOTHMAINNORMALWINDOW_H
