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

#include "bluetoothmainnormalwindow.h"

BluetoothMainNormalWindow::BluetoothMainNormalWindow(BlueToothDBusService * btServer,QWidget * parent) :
    m_btServer(btServer),
    QWidget(parent)
{
    this->setFocusPolicy(Qt::NoFocus);

    Init();
    InitConnectData();
}

BluetoothMainNormalWindow::~BluetoothMainNormalWindow()
{

}

void BluetoothMainNormalWindow::Init()
{
    _NormalWidgetMainLayout = new QVBoxLayout(this);
    _NormalWidgetMainLayout->setSpacing(40);
    _NormalWidgetMainLayout->setContentsMargins(0,0,0,10);

    m_topWidget = new BluetoothTopWindow(m_btServer,this);
    m_middleWidget = new BluetoothMiddleWindow(m_btServer,this);
    m_bottomWidget = new BluetoothBottomWindow(m_btServer,this);

    _NormalWidgetMainLayout->addWidget(m_topWidget,1,Qt::AlignTop);
    _NormalWidgetMainLayout->addWidget(m_middleWidget,1,Qt::AlignTop);
    _NormalWidgetMainLayout->addWidget(m_bottomWidget,1,Qt::AlignTop);
    _NormalWidgetMainLayout->addStretch(100);

    if (m_btServer && (nullptr != m_btServer->m_default_bluetooth_adapter))
        m_defaultPowerStatus = m_btServer->m_default_bluetooth_adapter->getAdapterPower();
    if (m_defaultPowerStatus)
    {
        QStringList devPaired_List=  BlueToothDBusService::getDefaultAdapterPairedDev();
        if( devPaired_List.count() == 0)
            m_middleWidget->setHidden(true);
        else
            m_middleWidget->setHidden(false);
        m_bottomWidget->setHidden(false);
    }
    else
    {
        m_middleWidget->setHidden(true);
        m_bottomWidget->setHidden(true);
    }
}

void BluetoothMainNormalWindow::InitDisplayStatus()
{
    if (m_btServer && (nullptr != m_btServer->m_default_bluetooth_adapter))
        m_defaultPowerStatus = m_btServer->m_default_bluetooth_adapter->getAdapterPower();
    if (m_defaultPowerStatus)
    {
        QStringList devPaired_List=  BlueToothDBusService::getDefaultAdapterPairedDev();
        if( devPaired_List.count() == 0)
            m_middleWidget->setHidden(true);
        else
            m_middleWidget->setHidden(false);
        m_bottomWidget->setHidden(false);
    }
    else
    {
        m_middleWidget->setHidden(true);
        m_bottomWidget->setHidden(true);
    }
}

void BluetoothMainNormalWindow::InitConnectData()
{
    if(m_btServer)
    {
        connect(m_btServer,&BlueToothDBusService::adapterPoweredChanged,this,&BluetoothMainNormalWindow::BluetoothSwitchChanged);
        connect(m_btServer,&BlueToothDBusService::defaultAdapterChangedSignal,this,&BluetoothMainNormalWindow::defaultAdapterChangedSlot);
    }

    if (m_topWidget)
    {
        connect(m_topWidget,&BluetoothTopWindow::btPowerSwitchChanged,this,&BluetoothMainNormalWindow::BluetoothSwitchChanged);
    }

    if (m_middleWidget)
    {
        connect(m_middleWidget,&BluetoothMiddleWindow::myDeviceWindosHiddenSignal,this,&BluetoothMainNormalWindow::setHiddenForMyDevice);
    }
}

void BluetoothMainNormalWindow::BluetoothSwitchChanged(bool status)
{
    m_defaultPowerStatus = status;
    QTimer::singleShot(30,this,[=]{
        SetHidden(!status);
    });
}

void BluetoothMainNormalWindow::defaultAdapterChangedSlot(int indx)
{
    KyDebug() << indx;
    QStringList devPaired_List=  BlueToothDBusService::getDefaultAdapterPairedDev();
    if(devPaired_List.count() == 0)
        m_middleWidget->setHidden(true);
    else
        m_middleWidget->setHidden(false);

    quitWindow();
    reloadWindow();
}

void BluetoothMainNormalWindow::SetHidden(bool status)
{
    QStringList devPaired_List=  BlueToothDBusService::getDefaultAdapterPairedDev();
    if( devPaired_List.count() == 0)
        m_middleWidget->setHidden(true);
    else
        m_middleWidget->setHidden(status);
    m_bottomWidget->setHidden(status);
}

void BluetoothMainNormalWindow::setHiddenForMyDevice(bool status)
{
    m_middleWidget->setHidden(status);
}

void BluetoothMainNormalWindow::reloadWindow()
{
    if (m_topWidget)
        m_topWidget->reloadWindow();
    if (m_middleWidget)
        m_middleWidget->reloadWindow();
    if (m_bottomWidget)
        m_bottomWidget->reloadWindow();
    InitDisplayStatus();
}

void BluetoothMainNormalWindow::quitWindow()
{
    if (m_topWidget)
        m_topWidget->quitWindow();
    if (m_middleWidget)
        m_middleWidget->quitWindow();
    if (m_bottomWidget)
        m_bottomWidget->quitWindow();
}
