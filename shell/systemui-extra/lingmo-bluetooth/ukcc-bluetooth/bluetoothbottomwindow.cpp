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

#include "bluetoothbottomwindow.h"
#include <QRandomGenerator>

typedef QPair<QString,qint16> fruit;
//设备显示排序
QVector <fruit> m_btDevSortList_vec;

BluetoothBottomWindow::BluetoothBottomWindow(BlueToothDBusService * btServer,QWidget * parent) :
    m_btServer(btServer),
    QWidget(parent)
{
    this->setFocusPolicy(Qt::NoFocus);

    InitNormalWidgetBottom();
    InitConnectionData();

    if (btServer->m_default_bluetooth_adapter)
        AddBluetoothDevices();
}

BluetoothBottomWindow::~BluetoothBottomWindow()
{

}

void BluetoothBottomWindow::InitNormalWidgetBottom()
{
    KyDebug();
    QVBoxLayout *_MNormalWidgetBottomLayout = new QVBoxLayout(this);
    _MNormalWidgetBottomLayout->setSpacing(8);
    _MNormalWidgetBottomLayout->setContentsMargins(0,0,0,0);

    QHBoxLayout *titleLayout = new QHBoxLayout(this);
    titleLayout->setSpacing(0);
    titleLayout->setContentsMargins(0,0,0,0);

    //~ contents_path /Bluetooth/Bluetooth Devices
    QLabel *OtherDevicesLabel = new QLabel(tr("Bluetooth Devices"),this);
    OtherDevicesLabel->resize(72,25);
    OtherDevicesLabel->setContentsMargins(16,0,10,0);

    titleLayout->addWidget(OtherDevicesLabel, 1, Qt::AlignLeft);

    _LoadIcon = new LoadingLabel(this);
    _LoadIcon->setFixedSize(16,16);
    _LoadIcon->setTimerStart();
    titleLayout->addWidget(_LoadIcon, 1, Qt::AlignLeft);

    _DevTypeSelectComboBox = new QComboBox(this);
    _DevTypeSelectComboBox->clear();
    _DevTypeSelectComboBox->addItems(devTypeSelectStrList);
    currentShowTypeFlag = _DEV_TYPE(_DevTypeSelectComboBox->currentIndex());

    titleLayout->addStretch(25);
    titleLayout->addWidget(_DevTypeSelectComboBox, 1, Qt::AlignRight);
    _MNormalWidgetBottomLayout->addLayout(titleLayout);

    _MNormalFrameBottom = new QFrame(this);
//    _MNormalFrameBottom->setMinimumWidth(582);
    _MNormalFrameBottom->adjustSize();
    _MNormalFrameBottom->setFrameShape(QFrame::Shape::Box);
    _MNormalFrameBottom->setContentsMargins(0,0,0,0);
    _MNormalWidgetBottomLayout->addWidget(_MNormalFrameBottom,1,Qt::AlignTop);

    _NormalWidgetCacheDevLayout = new QVBoxLayout(this);
    _NormalWidgetCacheDevLayout->setSpacing(0);
    _NormalWidgetCacheDevLayout->setContentsMargins(0,0,0,0);
    _NormalWidgetCacheDevLayout->setAlignment(Qt::AlignTop);
    _NormalWidgetCacheDevLayout->addStretch(10);
    _MNormalFrameBottom->setLayout(_NormalWidgetCacheDevLayout);

}

void BluetoothBottomWindow::InitConnectionData()
{
    connect(_DevTypeSelectComboBox,SIGNAL(currentIndexChanged(int)),this,SLOT(_DevTypeSelectComboBoxSlot(int)));

    if (m_btServer)
    {
        connect(m_btServer,&BlueToothDBusService::deviceAddSignal,this,&BluetoothBottomWindow::deviceAddSlot);
        connect(m_btServer,&BlueToothDBusService::deviceRemoveSignal,this,&BluetoothBottomWindow::deviceRemoveSlot);
        connect(m_btServer,&BlueToothDBusService::devicePairedSuccess,this,&BluetoothBottomWindow::devicePairedSuccessSlot);
        connect(m_btServer,&BlueToothDBusService::adapterDiscoveringChanged,this,&BluetoothBottomWindow::adapterDiscoveringSlot);
        connect(m_btServer,&BlueToothDBusService::defaultAdapterChangedSignal,this,&BluetoothBottomWindow::defaultAdapterChangedSlot);
    }
}

void BluetoothBottomWindow::AddBluetoothDevices()
{
    if (!BlueToothDBusService::m_default_bluetooth_adapter)
        return;

    QList<QString> devKeyList = BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list.keys(); // 存放的就是QMap的key值
    KyDebug() << devKeyList;
    for(QString dev_addr:devKeyList)
    {
        if(BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list.contains(dev_addr) &&
           !BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list[dev_addr]->isPaired())
            addOneBluetoothDeviceItemUi(dev_addr);
    }

    KyDebug() << m_btDevSortList_vec;

}

void BluetoothBottomWindow::addOneBluetoothDeviceItemUi(QString dev_address)
{
    KyDebug();
    //设备不存在
    if(!BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list.contains(dev_address))
    {
        KyDebug() << "device is NULL!";
        return ;
    }

    if(BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list[dev_address]->getDevName().isEmpty())
    {
        KyDebug() << "device is not name!";
        return ;
    }
    KyDebug() << dev_address
                << BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list[dev_address]->getDevName()
                << BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list[dev_address]->getDevRssi();

    //非法设备/和当前显示界面类型选择不一致
    if (!whetherToDisplayInTheCurrentInterface(BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list[dev_address]->getDevType()))
    {
        KyInfo() << "Dev Type Not!" ;
        return ;
    }

    //设备界面已存在
    bluetoothdevicewindowitem  * item  = this->findChild<bluetoothdevicewindowitem *>(dev_address);
    if (item)
    {
        KyDebug() << "device is exist" ;
        return;
    }

    int insert_index = getDevRssiItemInsertIndex(BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list[dev_address]->getDevRssi());
    KyDebug() << "insert_index:"<< insert_index ;

    if (insert_index == -1)
    {
        m_btDevSortList_vec.append(fruit(BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list[dev_address]->getDevAddress(),BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list[dev_address]->getDevRssi()));
        insert_index = _NormalWidgetCacheDevLayout->count();
    }
    else
    {
        m_btDevSortList_vec.insert(insert_index,fruit(BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list[dev_address]->getDevAddress(),BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list[dev_address]->getDevRssi()));
        if (insert_index > _NormalWidgetCacheDevLayout->count())
            insert_index = _NormalWidgetCacheDevLayout->count();
    }
    KyDebug() << "_NormalWidgetCacheDevLayout count:" << _NormalWidgetCacheDevLayout->count() ;
    KyDebug() << "m_btDevSortList_vec:" << m_btDevSortList_vec;

    bool showLine = true;
    if (_NormalWidgetCacheDevLayout->count() == 0)
        showLine =  false;
    item = new bluetoothdevicewindowitem(dev_address,showLine,this);//一直显示

    connect(item,&bluetoothdevicewindowitem::devRssiChanged,this,[=](qint64 value)
    {
        KyDebug() << item->objectName() << value ;
        //显示位置调整
        adjustDevItemDisplayPosition(item->objectName(),value);
    });
    connect(item,&bluetoothdevicewindowitem::bluetoothDeviceItemRemove,this,&BluetoothBottomWindow::deviceRemoveSlot);

    if (insert_index == _NormalWidgetCacheDevLayout->count())
    {
        item->setLineFrameHidden(true);
        setLastDevItemWindowLine(false);
    }
    else
    {
        item->setLineFrameHidden(false);
    }

    _NormalWidgetCacheDevLayout->insertWidget(insert_index,item,0,Qt::AlignTop);
}

bool BluetoothBottomWindow::whetherToDisplayInTheCurrentInterface(bluetoothdevice::DEVICE_TYPE devType)
{
    KyDebug() << devType << _DEV_TYPE(_DevTypeSelectComboBox->currentIndex());
    if (nullptr != _DevTypeSelectComboBox)
        currentShowTypeFlag = _DEV_TYPE(_DevTypeSelectComboBox->currentIndex());
    if(_DEV_TYPE::BT_All == currentShowTypeFlag)//显示为全部设备时，直接返回true
    {
        KyDebug() << "currentShowTypeFlag:" << currentShowTypeFlag ;
        return true;
    }

    _DEV_TYPE temp_dev_type ;

    if (devType == bluetoothdevice::DEVICE_TYPE::headset ||
        devType == bluetoothdevice::DEVICE_TYPE::headphones ||
        devType == bluetoothdevice::DEVICE_TYPE::audiovideo) {
        temp_dev_type = BT_Audio;
    }
    else if (devType == bluetoothdevice::DEVICE_TYPE::mouse ||
             devType == bluetoothdevice::DEVICE_TYPE::keyboard) {
        temp_dev_type = BT_Peripherals;

    }
    else if (devType == bluetoothdevice::DEVICE_TYPE::computer) {
        temp_dev_type = BT_Computer;
    }
    else if (devType == bluetoothdevice::DEVICE_TYPE::phone) {
        temp_dev_type = BT_Phone;
    }
    else
    {
        temp_dev_type = BT_Other;
    }

    KyInfo() << "currentShowTypeFlag:" << currentShowTypeFlag <<  "temp_dev_type:" << temp_dev_type ;

    if (currentShowTypeFlag == temp_dev_type)
    {
        KyInfo() << "dev type == : true" ;
        return true;
    }
    else
    {
        KyInfo() << "dev type == : false" ;;
        return false;
    }
}

int BluetoothBottomWindow::getDevRssiItemInsertIndex(qint16 currDevRssi)
{
    KyDebug() << "currDevRssi："<< currDevRssi ;
    //KyDebug () << "m_btDevSortList_vec:"<< m_btDevSortList_vec ;

    for(auto it = m_btDevSortList_vec.begin() ; it != m_btDevSortList_vec.end() ; ++it)
    {
        //KyDebug()  << "it->first(address):"<< it->first << "it->second(rssi):"<< it->second ;
        if (it->second <= currDevRssi)
        {
            //KyDebug() << "========index:"<< m_btDevSortList_vec.indexOf(fruit(it->first,it->second)) ;
            return (m_btDevSortList_vec.indexOf(fruit(it->first,it->second)));
        }
    }
    return -1;
}

void BluetoothBottomWindow::_DevTypeSelectComboBoxSlot(int indx)
{
    KyDebug() << indx;

    ukccbluetoothconfig::ukccBtBuriedSettings(QString("Bluetooth"),QString("DevTypeSelectComboBox"),QString("settings"),QString::number(indx));

    currentShowTypeFlag = _DEV_TYPE(indx);
    //清空列表数据
    QLayoutItem *child;

    while(_NormalWidgetCacheDevLayout->count()!=0)
    {
        child = _NormalWidgetCacheDevLayout->takeAt(0);
        if(child->widget() != 0)
        {
            delete child->widget();
        }
        delete child;
    }

    m_btDevSortList_vec.clear();

    reloadDeviceListItem(currentShowTypeFlag);

}


void BluetoothBottomWindow::reloadDeviceListItem(BluetoothBottomWindow::_DEV_TYPE flag)
{
    KyDebug() << flag;
    if (!BlueToothDBusService::m_default_bluetooth_adapter)
    {
        KyWarning() << "m_default_bluetooth_adapter is NULL!" ;
        return ;
    }
    //KyInfo() << BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list;
    QList<QString> devKeyList = BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list.keys(); // 存放的就是QMap的key值

    int count = 0;
    for (QString dev_addr : devKeyList)
    {
        if (BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list[dev_addr]->isPaired())
            continue;
        switch (flag) {
        case _DEV_TYPE::BT_All:
            if(count == 0)
            {
                addOneBluetoothDeviceItemUi(dev_addr);
            }
            else
            {
                QTimer::singleShot(50,this,[=]{
                    addOneBluetoothDeviceItemUi(dev_addr);
                });
            }
            count ++;
            break;
        case _DEV_TYPE::BT_Audio:
            if (BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list[dev_addr]->getDevType() == bluetoothdevice::DEVICE_TYPE::headset ||
                BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list[dev_addr]->getDevType() == bluetoothdevice::DEVICE_TYPE::headphones ||
                BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list[dev_addr]->getDevType() == bluetoothdevice::DEVICE_TYPE::audiovideo) {
                if(count == 0)
                {
                    addOneBluetoothDeviceItemUi(dev_addr);
                }
                else
                {
                    QTimer::singleShot(50,this,[=]{
                        addOneBluetoothDeviceItemUi(dev_addr);
                    });
                }
                count ++;
            }
            break;
        case _DEV_TYPE::BT_Peripherals:
            if (BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list[dev_addr]->getDevType() == bluetoothdevice::DEVICE_TYPE::mouse ||
                BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list[dev_addr]->getDevType() == bluetoothdevice::DEVICE_TYPE::keyboard) {
                if(count == 0)
                {
                    addOneBluetoothDeviceItemUi(dev_addr);
                }
                else
                {
                    QTimer::singleShot(50,this,[=]{
                        addOneBluetoothDeviceItemUi(dev_addr);
                    });
                }
                count ++;
            }
            break;
        case _DEV_TYPE::BT_Computer:
            if (BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list[dev_addr]->getDevType() == bluetoothdevice::DEVICE_TYPE::computer) {
                if(count == 0)
                {
                    addOneBluetoothDeviceItemUi(dev_addr);
                }
                else
                {
                    QTimer::singleShot(50,this,[=]{
                        addOneBluetoothDeviceItemUi(dev_addr);
                    });
                }
                count ++;
            }
            break;
        case _DEV_TYPE::BT_Phone:
            if (BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list[dev_addr]->getDevType() == bluetoothdevice::DEVICE_TYPE::phone) {
                if(count == 0)
                {
                    addOneBluetoothDeviceItemUi(dev_addr);
                }
                else
                {
                    QTimer::singleShot(50,this,[=]{
                        addOneBluetoothDeviceItemUi(dev_addr);
                    });
                }
                count ++;
            }
            break;
        case _DEV_TYPE::BT_Other:
            if (BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list[dev_addr]->getDevType() == bluetoothdevice::DEVICE_TYPE::headset ||
                BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list[dev_addr]->getDevType() == bluetoothdevice::DEVICE_TYPE::headphones ||
                BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list[dev_addr]->getDevType() == bluetoothdevice::DEVICE_TYPE::audiovideo ||
                BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list[dev_addr]->getDevType() == bluetoothdevice::DEVICE_TYPE::phone ||
                BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list[dev_addr]->getDevType() == bluetoothdevice::DEVICE_TYPE::mouse ||
                BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list[dev_addr]->getDevType() == bluetoothdevice::DEVICE_TYPE::keyboard ||
                BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list[dev_addr]->getDevType() == bluetoothdevice::DEVICE_TYPE::computer)
                break;
            else
            {
                if(count == 0)
                {
                    addOneBluetoothDeviceItemUi(dev_addr);
                }
                else
                {
                    QTimer::singleShot(50,this,[=]{
                        addOneBluetoothDeviceItemUi(dev_addr);
                    });
                }
                count ++;
            }
            break;
        default:
            if(count == 0)
            {
                addOneBluetoothDeviceItemUi(dev_addr);
            }
            else
            {
                QTimer::singleShot(50,this,[=]{
                    addOneBluetoothDeviceItemUi(dev_addr);
                });
            }
            count ++;
            break;
        }
    }
}

void BluetoothBottomWindow::deviceAddSlot(QString dev_address)
{
    KyDebug();

    if (BlueToothDBusService::m_default_bluetooth_adapter && BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list.contains(dev_address))
    {
        if (!BlueToothDBusService::m_default_bluetooth_adapter->m_bt_dev_list[dev_address]->isPaired())
        {
            if (_LoadIcon->isHidden())
            {
                _LoadIcon->show();
                _LoadIcon->setTimerStart();
            }

            KyDebug() << dev_address << ":dev is not pair!" ;
            addOneBluetoothDeviceItemUi(dev_address);
        }
    }
}

void BluetoothBottomWindow::removeBluetoothDeviceItemUi(QString address)
{
    KyDebug() << address ;
    //移除蓝牙设备列表的设备
    bluetoothdevicewindowitem *item = _MNormalFrameBottom->findChild<bluetoothdevicewindowitem *>(address);
    if(item)
    {
        KyDebug() << item->objectName();
        _NormalWidgetCacheDevLayout->removeWidget(item);
        item->disconnect();
        item->deleteLater();
    }
    else
    {
        KyDebug() << "bluetooth device list NULL!!";
    }

    for(auto it = m_btDevSortList_vec.begin(); it != m_btDevSortList_vec.end() ;++it)
    {
        if (it->first == address)
        {
            KyDebug() << m_btDevSortList_vec;
            m_btDevSortList_vec.removeAll(fruit(it->first,it->second));
            KyDebug()  << m_btDevSortList_vec;
            break;
        }
    }
    KyDebug() << "remove Item UI end" ;
}

void BluetoothBottomWindow::deviceRemoveSlot(QString dev_address)
{
    KyDebug() << dev_address;
    removeBluetoothDeviceItemUi(dev_address);
}

void BluetoothBottomWindow::devicePairedSuccessSlot(QString dev_address)
{
    KyDebug() << dev_address ;

    //判断其他设备中是否存在该设备界面
    bluetoothdevicewindowitem *other_item = this->findChild<bluetoothdevicewindowitem *>(dev_address);
    if (other_item)
        removeBluetoothDeviceItemUi(dev_address);
}

void BluetoothBottomWindow::adapterDiscoveringSlot(bool status)
{
    KyDebug() << status;

    _LoadIcon->setVisible(status);
    if (status)
        _LoadIcon->setTimerStart();
    else
        _LoadIcon->setTimerStop();
}

void BluetoothBottomWindow::adjustDevItemDisplayPosition(QString address,quint16 rssiValue)
{
    KyDebug()<< address << rssiValue ;
    //判断界面是否需要调整位置
    int old_devIndx = 0;
    for(auto it = m_btDevSortList_vec.begin() ; it != m_btDevSortList_vec.end() ; ++it)
    {
        if(it->first == address)
            break;
        old_devIndx++;
    }

    int new_devIndx = getDevRssiItemInsertIndex(rssiValue);
    //和原有位置做对比
    if (new_devIndx == old_devIndx ||  new_devIndx-1 == old_devIndx)
    {
        KyDebug() << "No change in position" ;
        return ;
    }

    bluetoothdevicewindowitem *new_item = this->findChild<bluetoothdevicewindowitem *>(address);
    if(new_item)
        _NormalWidgetCacheDevLayout->removeWidget(new_item);

    int countIndx = 0;
    for(auto it = m_btDevSortList_vec.begin() ; it != m_btDevSortList_vec.end() ; ++it)
    {
        if(it->first == address)
        {
            m_btDevSortList_vec.removeAt(countIndx);
            break;
        }
        countIndx++;
    }

    int insert_index = getDevRssiItemInsertIndex(rssiValue);
    //KyInfo()  << "insert_index:"<< insert_index ;

    if (insert_index == -1 || insert_index >= _NormalWidgetCacheDevLayout->count())
    {
        m_btDevSortList_vec.append(fruit(address,rssiValue));
        insert_index = _NormalWidgetCacheDevLayout->count();
    }
    else
    {
        m_btDevSortList_vec.insert(insert_index,fruit(address,rssiValue));
    }

    if (insert_index == _NormalWidgetCacheDevLayout->count())
    {
        new_item->setLineFrameHidden(true);
        setLastDevItemWindowLine(false);
    }
    else
    {
        new_item->setLineFrameHidden(false);
        setLastDevItemWindowLine(true);
    }
    _NormalWidgetCacheDevLayout->insertWidget(insert_index,new_item,0,Qt::AlignTop);
}

void BluetoothBottomWindow::defaultAdapterChangedSlot(int indx)
{
    KyDebug() << indx;
    reloadWindow();
}

void BluetoothBottomWindow::setLastDevItemWindowLine(bool status)
{
    //设置最后一个item line是否需要显示
    KyDebug() << "" << _NormalWidgetCacheDevLayout->count();
    if (_NormalWidgetCacheDevLayout->count() >= 1)
    {
        QLayoutItem *item = _NormalWidgetCacheDevLayout->itemAt(_NormalWidgetCacheDevLayout->count()-1);
        if (item->widget() != 0)
        {
            QString device_address = item->widget()->objectName();
            KyDebug() << device_address;
            bluetoothdevicewindowitem *dev_item  = _MNormalFrameBottom->findChild<bluetoothdevicewindowitem *>(device_address);
            if (dev_item)
                dev_item->setLineFrameHidden(status);
        }
    }
}

void BluetoothBottomWindow::clearOtherDevicesUI()
{
    KyWarning() << "Other dev count:" << _NormalWidgetCacheDevLayout->count();
    //清空列表数据
    QLayoutItem *child;
    while(_NormalWidgetCacheDevLayout->count()!=0)
    {
        child = _NormalWidgetCacheDevLayout->takeAt(0);
        if(child->widget() != 0)
        {
            delete child->widget();
        }
        delete child;
    }
    KyDebug() << "Other dev list（clean）:" << m_btDevSortList_vec ;
    m_btDevSortList_vec.clear();
}

void BluetoothBottomWindow::reloadWindow()
{    
    clearOtherDevicesUI();
    AddBluetoothDevices();
}

void BluetoothBottomWindow::quitWindow()
{
    clearOtherDevicesUI();
}
