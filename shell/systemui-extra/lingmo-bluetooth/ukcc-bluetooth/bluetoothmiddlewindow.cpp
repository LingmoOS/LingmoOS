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

#include "bluetoothmiddlewindow.h"

BluetoothMiddleWindow::BluetoothMiddleWindow(BlueToothDBusService * btServer,QWidget * parent):
    m_btServer(btServer),
    QWidget(parent)
{
    this->setFocusPolicy(Qt::NoFocus);

    InitNormalWidgetMiddle();
    InitConnectionData();

    if (btServer->m_default_bluetooth_adapter)
        AddMyBluetoothDevices();
}

BluetoothMiddleWindow::~BluetoothMiddleWindow()
{

}

void BluetoothMiddleWindow::InitNormalWidgetMiddle()
{
//    KyDebug();
    QVBoxLayout *NormalWidgetMiddleLayout = new QVBoxLayout(this);
    NormalWidgetMiddleLayout->setSpacing(8);
    NormalWidgetMiddleLayout->setContentsMargins(0,0,0,0);

    //~ contents_path /Bluetooth/My Devices
    QLabel *MyDevicesLabel = new QLabel(tr("My Devices"),this);
    MyDevicesLabel->resize(72,25);
    MyDevicesLabel->setContentsMargins(16,0,0,0);
    NormalWidgetMiddleLayout->addWidget(MyDevicesLabel,Qt::AlignTop);

    _MNormalFrameMiddle = new QFrame(this);
    _MNormalFrameMiddle->setMinimumWidth(582);
    _MNormalFrameMiddle->setFrameShape(QFrame::Shape::Box);
    _MNormalFrameMiddle->setContentsMargins(0,0,0,0);
    NormalWidgetMiddleLayout->addWidget(_MNormalFrameMiddle,1,Qt::AlignTop);

    _NormalWidgetPairedDevLayout = new QVBoxLayout(_MNormalFrameMiddle);
    _NormalWidgetPairedDevLayout->setSpacing(0);
    _NormalWidgetPairedDevLayout->setContentsMargins(0,0,0,0);
    _MNormalFrameMiddle->setLayout(_NormalWidgetPairedDevLayout);

    _MConnectedNormalFrameMiddle = new QWidget(_MNormalFrameMiddle);
    _MConnectedNormalFrameMiddle->setMinimumWidth(582);
//    _MConnectedNormalFrameMiddle->setFrameShape(QFrame::Shape::Box);
    _MConnectedNormalFrameMiddle->setContentsMargins(0,0,0,0);
    _NormalWidgetPairedDevLayout->addWidget(_MConnectedNormalFrameMiddle);

    _MNotConnectedNormalFrameMiddle = new QWidget(_MNormalFrameMiddle);
    _MNotConnectedNormalFrameMiddle->setMinimumWidth(582);
//    _MNotConnectedNormalFrameMiddle->setFrameShape(QFrame::Shape::Box);
    _MNotConnectedNormalFrameMiddle->setContentsMargins(0,0,0,0);
    _NormalWidgetPairedDevLayout->addWidget(_MNotConnectedNormalFrameMiddle);

    _NormalWidgetConnecededDevLayout = new QVBoxLayout(_MNormalFrameMiddle);
    _NormalWidgetConnecededDevLayout->setSpacing(0);
    _NormalWidgetConnecededDevLayout->setContentsMargins(0,0,0,0);
    _MConnectedNormalFrameMiddle->setLayout(_NormalWidgetConnecededDevLayout);

    _NormalWidgetNotConnecededDevLayout = new QVBoxLayout(_MNotConnectedNormalFrameMiddle);
    _NormalWidgetNotConnecededDevLayout->setSpacing(0);
    _NormalWidgetNotConnecededDevLayout->setContentsMargins(0,0,0,0);
    _MNotConnectedNormalFrameMiddle->setLayout(_NormalWidgetNotConnecededDevLayout);
}
void BluetoothMiddleWindow::InitConnectionData()
{

    if (m_btServer)
    {
        connect(m_btServer,&BlueToothDBusService::deviceRemoveSignal,this,&BluetoothMiddleWindow::deviceRemoveSlot);
        connect(m_btServer,&BlueToothDBusService::devicePairedSuccess,this,&BluetoothMiddleWindow::devicePairedSuccessSlot);
        connect(m_btServer,&BlueToothDBusService::defaultAdapterChangedSignal,this,&BluetoothMiddleWindow::defaultAdapterChangedSlot);
    }
}


//void BluetoothMiddleWindow::AddMyDeviceWidgetConnectedAndNotConnected()
//{
//    //测试代码
//}

void BluetoothMiddleWindow::AddMyBluetoothDevices()
{
    if (!BlueToothDBusService::m_default_bluetooth_adapter)
        return;
    QList<QString> pairedDevKeyList = BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list.keys(); // 存放的就是QMap的key值
    KyDebug() << pairedDevKeyList;
    for (QString dev_addr:pairedDevKeyList)
    {
        if (BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list[dev_addr]->isPaired())
            addMyDeviceItemUI(dev_addr);
    }
}


void BluetoothMiddleWindow::addMyDeviceItemUI(QString device_address)
{
    KyDebug() << device_address;

    bluetoothdevicewindowitem *item  = this->findChild<bluetoothdevicewindowitem *>(device_address);
    if (item)
    {
        KyInfo() << device_address << ":device is exist" ;
        return;
    }

    bool show_line = true;
    if (_NormalWidgetConnecededDevLayout->count() == 0 && _NormalWidgetNotConnecededDevLayout->count() == 0)
        show_line = false;

    item = new bluetoothdevicewindowitem(device_address,show_line,this);
    if (item)
    {
        connect(item,&bluetoothdevicewindowitem::devConnectedChanged,this,[=](bool status){
            devConnectedChangedSlot(device_address,status);
        });
        connect(item,&bluetoothdevicewindowitem::bluetoothDeviceItemRemove,this,&BluetoothMiddleWindow::deviceRemoveSlot);

        if (BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list[device_address]->isConnected())
        {
            item->setParent(_MConnectedNormalFrameMiddle);
            _NormalWidgetConnecededDevLayout->insertWidget(0,item,1,Qt::AlignTop);
        }
        else
        {
            item->setParent(_MNotConnectedNormalFrameMiddle);
            if(_NormalWidgetNotConnecededDevLayout->count() == 0)
            {
                item->setLineFrameHidden(true);
                setLastDevItemWindowLine(false);
            }
//            if (bluetoothdevice::DEVICE_TYPE::mouse == BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list[device_address]->getDevType() ||
//                bluetoothdevice::DEVICE_TYPE::keyboard == BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list[device_address]->getDevType())
//            {
//                setLastDevItemWindowLine(false);
//                item->setEnabled(false);
//                _NormalWidgetNotConnecededDevLayout->addWidget(item);
//            }
//            else
            _NormalWidgetNotConnecededDevLayout->insertWidget(0,item,1,Qt::AlignTop);
        }
    }
}

void BluetoothMiddleWindow::removeMyDeviceItemUI(QString device_address)
{
    KyDebug() <<  device_address;
    bluetoothdevicewindowitem *item  = _MNotConnectedNormalFrameMiddle->findChild<bluetoothdevicewindowitem *>(device_address);
    if (item)
    {
        _NormalWidgetNotConnecededDevLayout->removeWidget(item);
        item->disconnect();
        item->deleteLater();
    }

    item  = _MConnectedNormalFrameMiddle->findChild<bluetoothdevicewindowitem *>(device_address);
    if (item)
    {
        _NormalWidgetConnecededDevLayout->removeWidget(item);
        item->disconnect();
        item->deleteLater();
    }

    if (_NormalWidgetNotConnecededDevLayout->count() == 0 && _NormalWidgetConnecededDevLayout->count() == 0)
        Q_EMIT myDeviceWindosHiddenSignal(true);
    else
    {
        setLastDevItemWindowLine(true);
    }
}

void BluetoothMiddleWindow::setLastDevItemWindowLine(bool status)
{
    //设置最后一个item line是否需要显示
    KyDebug() << "" << _NormalWidgetNotConnecededDevLayout->count();
    if (_NormalWidgetNotConnecededDevLayout->count() >= 1)
    {
        QLayoutItem *item = _NormalWidgetNotConnecededDevLayout->itemAt(_NormalWidgetNotConnecededDevLayout->count()-1);
        if (item->widget() != 0)
        {
            QString device_address = item->widget()->objectName();
            KyDebug() << device_address;
            bluetoothdevicewindowitem *dev_item  = _MNotConnectedNormalFrameMiddle->findChild<bluetoothdevicewindowitem *>(device_address);
            if (dev_item)
                dev_item->setLineFrameHidden(status);
        }
        return;
    }

    KyDebug() << "" << _NormalWidgetConnecededDevLayout->count();

    if (_NormalWidgetConnecededDevLayout->count() >= 1)
    {
        QLayoutItem *item = _NormalWidgetConnecededDevLayout->itemAt(_NormalWidgetConnecededDevLayout->count()-1);
        if (item->widget() != 0)
        {
            QString device_address = item->widget()->objectName();
            KyDebug() << device_address;
            bluetoothdevicewindowitem *dev_item  = _MConnectedNormalFrameMiddle->findChild<bluetoothdevicewindowitem *>(device_address);
            if (dev_item)
                dev_item->setLineFrameHidden(status);
        }
    }
}

void BluetoothMiddleWindow::deviceRemoveSlot(QString dev_address)
{
    KyDebug() << dev_address;
    removeMyDeviceItemUI(dev_address);
}

void BluetoothMiddleWindow::devConnectedChangedSlot(QString dev_address,bool status)
{
    KyDebug() << dev_address << " ConnectedChanged :" << status;
    bluetoothdevicewindowitem *item  = _MNormalFrameMiddle->findChild<bluetoothdevicewindowitem *>(dev_address);
    if (item)
    {
        KyDebug() << dev_address << "==== ConnectedChanged :" << status;

        if (status)
        {
            _NormalWidgetNotConnecededDevLayout->removeWidget(item);

            item->setParent(_MConnectedNormalFrameMiddle);
            if ((_NormalWidgetConnecededDevLayout->count() == 0) && (_NormalWidgetNotConnecededDevLayout->count() == 0))
                item->setLineFrameHidden(true);
            else
                item->setLineFrameHidden(false);

            setLastDevItemWindowLine(true);
            _NormalWidgetConnecededDevLayout->insertWidget(0,item,1,Qt::AlignTop);
        }
        else
        {
            _NormalWidgetConnecededDevLayout->removeWidget(item);

            item->setParent(_MNotConnectedNormalFrameMiddle);
            if (_NormalWidgetNotConnecededDevLayout->count() == 0)
            {
                item->setLineFrameHidden(true);
                setLastDevItemWindowLine(false);
            }
            else
                item->setLineFrameHidden(false);

            _NormalWidgetNotConnecededDevLayout->insertWidget(0,item,1,Qt::AlignTop);
        }
    }
}

void BluetoothMiddleWindow::devicePairedSuccessSlot(QString dev_address)
{
    if (this->isHidden())
        Q_EMIT myDeviceWindosHiddenSignal(false); ;
    addMyDeviceItemUI(dev_address);
}

void BluetoothMiddleWindow::defaultAdapterChangedSlot(int indx)
{
    KyDebug() << indx;
    quitWindow();
    reloadWindow();
}

void BluetoothMiddleWindow::clearMyDevicesUI()
{
    KyDebug() << "Connected dev count :" << _NormalWidgetConnecededDevLayout->count()
              << "Paired dev count :" << _NormalWidgetNotConnecededDevLayout->count();
    //清空连接成功列表数据
    QLayoutItem *child;
    while(_NormalWidgetConnecededDevLayout->count()!=0)
    {
        child = _NormalWidgetConnecededDevLayout->takeAt(0);
        if(child->widget() != 0)
        {
            delete child->widget();
        }
        delete child;
    }

    //清空未连接列表数据
    while(_NormalWidgetNotConnecededDevLayout->count()!=0)
    {
        child = _NormalWidgetNotConnecededDevLayout->takeAt(0);
        if(child->widget() != 0)
        {
            delete child->widget();
        }
        delete child;
    }
}

void BluetoothMiddleWindow::reloadWindow()
{
    clearMyDevicesUI();
    AddMyBluetoothDevices();
}

void BluetoothMiddleWindow::quitWindow()
{
    clearMyDevicesUI();
}
