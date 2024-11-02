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

#ifndef BLUETOOTHTOPWINDOW_H
#define BLUETOOTHTOPWINDOW_H

#include <QLabel>
#include <QFrame>
#include <QWidget>
#include <QComboBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>

#include "kswitchbutton.h"
#include "bluetoothnamelabel.h"
#include "bluetoothdbusservice.h"

#define MIN_WIDTH   582
#define MIN_HEIGTH  58

using namespace kdk;


class BluetoothTopWindow : public QWidget
{
    Q_OBJECT
public:
    BluetoothTopWindow(BlueToothDBusService * btServer,QWidget * parent = nullptr);
    ~BluetoothTopWindow();

    static bool m_defaultAdapterPowerStatus;

    void reloadWindow();
    void quitWindow();

Q_SIGNALS:
    void btPowerSwitchChanged(bool status);

private Q_SLOTS:
    void setDefaultAdapterNameSlot(QString);

    void _BtSwitchBtnSlot(bool status);
    void _BtSwitchBtnPressedSlot();
    void _BtSwitchBtnReleasedSlot();

    void _BtTrayIconShowSlot(bool status);
    void _BtDiscoverableSlot(bool status);
    void _BtAutoAudioConnBtnSlot(bool status);
    void _AdapterListSelectComboBoxSlot(int indx);

    void adapterAddSlot(QString);
    void adapterRemoveSlot(int);
    void defaultAdapterChangedSlot(int);

    void adapterNameChangedSlot(QString);
    void adapterNameChangedOfIndexSlot(int indx , QString name);
    void adapterPowerStatusChangedSlot(bool);
    void adapterTrayIconSlot(bool);
    void adapterDiscoverableSlot(bool);
    void adapterActiveConnectionSlot(bool);

private:

    BlueToothDBusService * m_btServer = nullptr;

    QFrame  *_MNormalFrameTop = nullptr;

    QFrame  *BtSwitchLineFrame = nullptr;
    QFrame  *BtAdapterListFrame = nullptr;
    QFrame  *BtAdapterListLineFrame = nullptr;
    QFrame  *BtTrayIconShowFrame= nullptr;
    QFrame  *BtTrayIconShowLineFrame = nullptr;
    QFrame  *BtDiscoverableFrame= nullptr;
    QFrame  *BtDiscoverableLineFrame= nullptr;
    QFrame  *BtAutomaticAudioConnectionFrame= nullptr;

    KSwitchButton   *_BtSwitchBtn = nullptr;
    KSwitchButton   *_BtTrayIconShow = nullptr;
    KSwitchButton   *_BtDiscoverable = nullptr;
    KSwitchButton   *_BtAutoAudioConnBtn = nullptr;

    QComboBox       *_AdapterListSelectComboBox = nullptr;

    BluetoothNameLabel  *_BtNameLabel = nullptr;

    bool _BTServiceReportPowerSwitchFlag = false;           //服务上报的蓝牙开关状态
    bool _BTServiceReportTrayIconSwitchFlag = false;        //服务上报的蓝牙托盘显示开关状态
    bool _BTServiceReportDiscoverableSwtichFlag = false;    //服务上报的可被发现状态
    bool _BTServiceReportAutoAudioConnSwtichFlag = false;    //服务上报的自动音频状态
//    bool _BTServiceReportDiscoveringSwtichFlag = false;     //服务上报的扫描状态

    bool m_SwitchBtnPressed = false;

private:
    void InitNormalWidgetTop();
    void InitDisplayState();
    void InitConnectionData();
    void sendBtPowerChangedSignal(bool status);
    void adapterChangedRefreshInterface(int indx);

    bool whetherNeedInfoUser();
};

#endif // BLUETOOTHTOPWINDOW_H
