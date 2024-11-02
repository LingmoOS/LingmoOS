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

#include "bluetoothmainwindow.h"

typedef QPair<QString,qint16> fruit;
//设备显示排序
QVector <fruit> devShowOrderListVec;
bool BlueToothMainWindow::m_device_operating = false;

BlueToothMainWindow::BlueToothMainWindow(QWidget *parent):
    QMainWindow(parent)
{
    KyDebug();

    btServer = new BlueToothDBusService(this);
    m_init_btServer_res = 0 ;
    if (btServer)
    {
        m_init_btServer_res = btServer->initBluetoothServer();
        InitBTServerConnection();
    }
    else
        m_init_btServer_res = 1;

    KyInfo() << "res:1-- init Bluetooth Server failed!";
    KyInfo() << "res:2-- Bluetooth adapter is null!";
    KyInfo() <<  "ukcc start -- bluetooth server res:" << m_init_btServer_res ;

    InitWindows();
    InitLoadingTimer();

    if (0 == m_init_btServer_res)
    {
        displayNormalWindow();
    }
    else if (1 == m_init_btServer_res)
    {
        displayErrorBluetoothServerInitFailedWindow();
    }
    else if (2 == m_init_btServer_res)
    {
        displayErrorNoAdapterWindow();
    }
    else
    {
        displayErrorUnknownWindow();
    }
}

BlueToothMainWindow::~BlueToothMainWindow()
{
    KyDebug();

    if (btServer)
        btServer->deleteLater();
    if(m_loadingTimer)
        m_loadingTimer->deleteLater();
}



void BlueToothMainWindow::InitBTServerConnection()
{
    if (btServer)
    {
        connect(btServer,&BlueToothDBusService::adapterAddSignal,this,&BlueToothMainWindow::adapterAddSlot);
        connect(btServer,&BlueToothDBusService::adapterRemoveSignal,this,&BlueToothMainWindow::adapterRemoveSlot);
        connect(btServer,&BlueToothDBusService::defaultAdapterChangedSignal,this,&BlueToothMainWindow::defaultAdapterChangedSlot);

        //bt service update
        connect(btServer,&BlueToothDBusService::btServiceRestart,this,&BlueToothMainWindow::btServiceRestartSlot);
        //connect(btServer,&BlueToothDBusService::btServiceRestartComplete,this,&BlueToothMainWindow::btServiceRestartCompleteSlot);
    }
}

void BlueToothMainWindow::InitWindows()
{
    m_centralWidget = new QStackedWidget(this);
    this->setCentralWidget(m_centralWidget);

    InitNormalWindow();
    InitErrorWindow();
    InitLoadingWindow();

    m_centralWidget->insertWidget(MAINWINDOW_ERROR_WINDOW,m_errorWindow);
    m_centralWidget->insertWidget(MAINWINDOW_LOADING_INTERFACE,m_loadingWindow);
    m_centralWidget->insertWidget(MAINWINDOW_NORMAL_INTERFACE,m_normalWindow);
}

void BlueToothMainWindow::InitNormalWindow()
{
    m_normalWindow = new BluetoothMainNormalWindow(btServer,m_centralWidget);
}

void BlueToothMainWindow::InitErrorWindow()
{
    m_errorWindow = new BluetoothMainErrorWindow(tr("Bluetooth Adapter loading Failed!"), m_centralWidget);
}

void BlueToothMainWindow::InitLoadingWindow()
{
    m_loadingWindow = new BluetoothMainLoadingWindow();
}

void BlueToothMainWindow::displayNormalWindow()
{
    KyInfo();
    if (m_btServer_errId != 0 || m_loadingTimer->isActive())
    {
        m_btServer_errId = 0;
        m_loadingTimer->stop();
    }
    m_centralWidget->setCurrentIndex(MAINWINDOW_NORMAL_INTERFACE);
}

void BlueToothMainWindow::displayErrorAbnormalWindow()
{
    m_normalWindow->SetHidden(true);
    if (m_errorWindow)
        m_errorWindow->setErrorText(tr("Bluetooth adapter is abnormal!"));
    m_centralWidget->setCurrentIndex(MAINWINDOW_ERROR_WINDOW);}

void BlueToothMainWindow::displayErrorNoAdapterWindow()
{
    m_normalWindow->SetHidden(true);
    if (m_errorWindow)
        m_errorWindow->setErrorText(tr("No Bluetooth adapter detected!"));
    m_centralWidget->setCurrentIndex(MAINWINDOW_ERROR_WINDOW);
}

void BlueToothMainWindow::displayErrorBluetoothServerInitFailedWindow()
{
    m_normalWindow->SetHidden(true);
    if (m_errorWindow)
        m_errorWindow->setErrorText(tr("Bluetooth Service init failed!"));
    m_centralWidget->setCurrentIndex(MAINWINDOW_ERROR_WINDOW);
}

void BlueToothMainWindow::displayErrorUnknownWindow()
{
    m_normalWindow->SetHidden(true);
    if (m_errorWindow)
        m_errorWindow->setErrorText(tr("Unknown Bluetooth Error!"));
    m_centralWidget->setCurrentIndex(MAINWINDOW_ERROR_WINDOW);
}

void BlueToothMainWindow::displayLoadingWindow()
{
    m_normalWindow->SetHidden(true);
    m_centralWidget->setCurrentIndex(MAINWINDOW_LOADING_INTERFACE);
    if (m_loadingTimer->isActive())
        m_loadingTimer->stop();
    m_loadingTimer->start();
}

void BlueToothMainWindow::InitLoadingTimer()
{
    //加载蓝牙适配器
    m_loadingTimer = new QTimer(this);
    m_loadingTimer->setInterval(6000);
    connect(m_loadingTimer,&QTimer::timeout,this,&BlueToothMainWindow::loadingTimeOutSlot);
}

void BlueToothMainWindow::loadingTimeOutSlot()
{
    if (m_loadingTimer->isActive())
        m_loadingTimer->stop();

    if (1 == this->m_errType_id)
        displayErrorNoAdapterWindow();
    else if  (2 == this->m_errType_id)
        displayErrorBluetoothServerInitFailedWindow();
    else if  (3 == this->m_errType_id)
        displayErrorAbnormalWindow();
    else
        displayErrorUnknownWindow();
}

void BlueToothMainWindow::adapterAddSlot(QString adapter_name)
{
    KyDebug() << "= adapter_name:" << adapter_name
              << "= BlueToothDBusService::m_bluetooth_adapter_address_list:" << BlueToothDBusService::m_bluetooth_adapter_address_list
              << "= list size :" << BlueToothDBusService::m_bluetooth_adapter_address_list.size() ;
    if (MAINWINDOW_NORMAL_INTERFACE!= this->m_centralWidget->currentIndex())
    {
        QTimer::singleShot(500,this,[=]{
            m_normalWindow->reloadWindow();
            displayNormalWindow();
        });
    }
}

void BlueToothMainWindow::adapterRemoveSlot(int indx)
{
    KyDebug() << "remove index:" << indx
              << "name list size :" << BlueToothDBusService::m_bluetooth_adapter_name_list.size()
              << "name list:" << BlueToothDBusService::m_bluetooth_adapter_name_list;

    if (0 == BlueToothDBusService::m_bluetooth_adapter_name_list.size())
    {
        this->m_errType_id = 1;
        displayLoadingWindow();
    }
}

void BlueToothMainWindow::btServiceRestartSlot()
{
    KyDebug();
    displayLoadingWindow();
}

void BlueToothMainWindow::btServiceRestartCompleteSlot(bool status)
{
    KyDebug() << status;

    if (m_loadingTimer->isActive())
        m_loadingTimer->stop();

    if (status)
    {
        if(MAINWINDOW_NORMAL_INTERFACE != this->m_centralWidget->currentIndex())
        {
            m_normalWindow->reloadWindow();
            displayNormalWindow();
        }
    }
    else
    {
        if(MAINWINDOW_LOADING_INTERFACE == this->m_centralWidget->currentIndex())
        {
            displayErrorBluetoothServerInitFailedWindow();
        }
    }
}

void BlueToothMainWindow::defaultAdapterChangedSlot(int indx)
{
    if (BlueToothMainWindow::m_device_operating)
        BlueToothMainWindow::m_device_operating = false;
}
