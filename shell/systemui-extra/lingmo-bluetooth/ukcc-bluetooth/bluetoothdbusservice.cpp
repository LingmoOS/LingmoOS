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

#include "bluetoothdbusservice.h"

QDBusInterface BlueToothDBusService::interface(DBUSSERVICE,DBUSPATH,DBUSINTERFACE);

bool BlueToothDBusService::m_taskbar_show_mark = true;
QStringList BlueToothDBusService::m_bluetooth_adapter_name_list ;
QStringList BlueToothDBusService::m_bluetooth_adapter_address_list ;
bluetoothadapter * BlueToothDBusService::m_default_bluetooth_adapter = nullptr;
QStringList BlueToothDBusService::m_bluetooth_Paired_Device_address_list;
QStringList BlueToothDBusService::m_bluetooth_All_Device_address_list;
QMap<QString, QVariant> BlueToothDBusService::defaultAdapterDataAttr;
QMap<QString, QVariant> BlueToothDBusService::deviceDataAttr;

BlueToothDBusService::BlueToothDBusService(QObject *parent):QObject(parent)
{
    KyDebug();
    defaultAdapterDataAttr.clear();
    deviceDataAttr.clear();

    m_loading_dev_timer = new QTimer(this);
    m_loading_dev_timer->setInterval(1000);
    connect(m_loading_dev_timer,SIGNAL(timeout()),this,SLOT(devLoadingTimeoutSlot()));

    bindServiceReportData();
}

BlueToothDBusService::~BlueToothDBusService()
{
    //向蓝牙注销
    unregisterClient();
    if(m_loading_dev_timer)
        m_loading_dev_timer->deleteLater();
}

int BlueToothDBusService::checkAddrList(QStringList &addrList)
{
    KyDebug() << addrList;
    if(addrList.size() <= 0)
        return 1;//没有适配器

    for (QString var : addrList) {
        if (6 == var.split(":").size())
            return 0;
    }
    return 2;//没有效的适配器地址
}

int BlueToothDBusService::initBluetoothServer()
{
    KyDebug();
    //注册
    QMap<QString, QVariant> value;
    value["dbusid"] = QDBusConnection::systemBus().baseService();
    //value["username"] = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    value["username"] = QString(qgetenv("USER").toStdString().data());
    value["type"] = int(0);

    KyWarning() << QString(qgetenv("USER").toStdString().data());

    QMap<QString, QVariant> reg_flag = registerClient(value);
    KyWarning() << reg_flag;
    if (!reg_flag.contains("result") &&  !reg_flag["result"].toBool())
    {//注册失败
        KyWarning() << "registerClient failed!" ;
        return 1;
    }

    if (reg_flag.contains("envPC"))
    {
        envPC = Environment(reg_flag["envPC"].toInt());
    }
    else
        ukccbluetoothconfig::setEnvPCValue();

    KyDebug();

    //获取适配器列表
    m_bluetooth_adapter_list.clear();
    m_bluetooth_adapter_name_list.clear();
    m_bluetooth_adapter_address_list.clear();

    m_bluetooth_adapter_address_list = getAllAdapterAddress();
    KyInfo() << m_bluetooth_adapter_address_list ;
    //int res = checkAddrList(m_bluetooth_adapter_address_list);
    if (m_bluetooth_adapter_address_list.isEmpty())
    {
        KyWarning() << "bluetooth adapter isEmpty" ;
        return 2;
    }

    //获取所有适配器数据
    for (auto var : m_bluetooth_adapter_address_list)
        getAdapterAllData(var);

    KyDebug() << "m_bluetooth_adapter_name_list:" << m_bluetooth_adapter_name_list
              << "m_bluetooth_adapter_address_list:" << m_bluetooth_adapter_address_list
              << "m_bluetooth_adapter_list:" << m_bluetooth_adapter_list ;

    return 0;//初始化正常返回0
}

QMap<QString, QVariant> BlueToothDBusService::registerClient(void)
{
    KyDebug();
    //注册
    QMap<QString, QVariant> value;
    value["dbusid"] = QDBusConnection::systemBus().baseService();
    //value["username"] = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    value["username"] = QString(qgetenv("USER").toStdString().data());
    value["type"] = int(0);
    QMap<QString, QVariant> reg_flag = registerClient(value);

    return reg_flag;
}

QMap<QString, QVariant> BlueToothDBusService::registerClient(QMap<QString, QVariant> value)
{
    KyDebug() << value ;
    QDBusReply<QMap<QString,QVariant>> reply_res;

    QDBusInterface iface(SERVICE,
                         PATH,
                         INTERFACE,
                         QDBusConnection::systemBus());

    QDBusPendingCall pcall = iface.asyncCall("registerClient", value);
    pcall.waitForFinished();

    QDBusMessage res = pcall.reply();

    if(res.type() == QDBusMessage::ReplyMessage)
    {
        if(res.arguments().size() > 0)
        {
            reply_res = res;
            KyInfo() << reply_res.value();
        }
    }
    else
    {
       KyWarning()<< res.errorName() << ": "<< res.errorMessage();
    }

    return reply_res.value();
}

bool BlueToothDBusService::unregisterClient()
{
    KyInfo();
    QDBusMessage message = QDBusMessage::createMethodCall(SERVICE,
                                                          PATH,
                                                          INTERFACE,
                                                          "unregisterClient");
    message << QDBusConnection::systemBus().baseService();
    QDBusReply<bool> reply_res = QDBusConnection::systemBus().call(message);
    return reply_res;
}

void BlueToothDBusService::reportUpdateClient()
{
    KyInfo();
    Q_EMIT btServiceRestart();
    int res = initBluetoothServer();
    if (0 == res)
        Q_EMIT btServiceRestartComplete(true);
    else
        Q_EMIT btServiceRestartComplete(false);
}

QStringList BlueToothDBusService::getAllAdapterAddress()
{
    KyDebug();
    QStringList reply_res;
    QDBusInterface iface(SERVICE,
                         PATH,
                         INTERFACE,
                         QDBusConnection::systemBus());

    QDBusPendingCall pcall = iface.asyncCall("getAllAdapterAddress");
    pcall.waitForFinished();

    QDBusMessage res = pcall.reply();
    if(res.type() == QDBusMessage::ReplyMessage)
    {
       if(res.arguments().size() > 0)
       {
           reply_res = res.arguments().takeFirst().toStringList();
           KyInfo() << reply_res;
       }
    }
    else
    {
       KyWarning()<< res.errorName() << ": "<< res.errorMessage();
    }

    return reply_res;
}

void BlueToothDBusService::getDefaultAdapterDevices()
{
    KyDebug();
    if (nullptr == m_default_bluetooth_adapter)
    {
        KyDebug() << "m_default_bluetooth_adapter is nullptr!" ;
        return;
    }
    m_default_bluetooth_adapter->m_bt_dev_list.clear();
//    m_default_bluetooth_adapter->m_bt_dev_paired_list.clear();

    //读取默认适配器下配对和扫描出的设备
    m_bluetooth_Paired_Device_address_list.clear();
    m_bluetooth_Paired_Device_address_list = getDefaultAdapterPairedDev();

    KyDebug () << m_bluetooth_Paired_Device_address_list;
    for (QString dev_addr : m_bluetooth_Paired_Device_address_list)
    {
        bluetoothdevice * dev = createOneBleutoothDeviceForAddress(dev_addr);
        if (dev != nullptr)
        {
            KyDebug () << "==========" << dev_addr << "ok";
            m_default_bluetooth_adapter->m_bt_dev_list.insert(dev_addr,dev);
//            m_default_bluetooth_adapter->m_bt_dev_paired_list[dev_addr] = dev;
            KyDebug () << m_default_bluetooth_adapter->m_bt_dev_list;
        }
    }

    int dev_add_count = 0;
    m_remainder_loaded_bluetooth_device_address_list.clear();
    m_bluetooth_All_Device_address_list.clear();
    m_bluetooth_All_Device_address_list = getDefaultAdapterAllDev();
    for (QString dev_addr : m_bluetooth_All_Device_address_list)
    {
        dev_add_count++;
        KyInfo()  << "add an device (mac):"  << dev_addr << "is count device: " << dev_add_count ;

//        if (dev_add_count > 100)
//        {//首次启动时,设备数量超过100个的情况下，多余的设备以定时加入的方式
//            m_remainder_loaded_bluetooth_device_address_list.append(dev_addr);
//        }
//        else
//        {
            bluetoothdevice * dev = createOneBleutoothDeviceForAddress(dev_addr);
            if (dev != nullptr && !dev->isPaired())
            {
                m_default_bluetooth_adapter->m_bt_dev_list.insert(dev->getDevAddress(),dev);
            }
//        }
    }

    KyInfo() << "####" << m_remainder_loaded_bluetooth_device_address_list;

//    if (m_remainder_loaded_bluetooth_device_address_list.size() > 0)
//    {
//        if (m_loading_dev_timer->isActive())
//            m_loading_dev_timer->stop();

//        m_loading_dev_timer->start();
//        KyInfo() << "start m_loading_dev_timer!" << m_remainder_loaded_bluetooth_device_address_list;
//    }

    KyDebug() << "end";
}

int BlueToothDBusService::getAdapterAllData(QString address)
{
    KyDebug() << address;
    QString dev_name,
            dev_address;
    bool    dev_block ,
            dev_power,
            dev_pairing     ,
            dev_pairable    ,
            dev_connecting  ,
            dev_discovering ,
            dev_discoverable,
            dev_activeConnection,
            dev_defaultAdapterMark,
            dev_trayShow;
    QMap<QString, QVariant> adpater_data = getAdapterAttr(address,"");
    KyInfo()<< " ===================== " << adpater_data;

    bluetoothAdapterDataAnalysis(adpater_data,
                                 dev_name,
                                 dev_address,
                                 dev_block ,
                                 dev_power,
                                 dev_pairing     ,
                                 dev_pairable    ,
                                 dev_connecting  ,
                                 dev_discovering ,
                                 dev_discoverable,
                                 dev_activeConnection,
                                 dev_defaultAdapterMark,
                                 dev_trayShow);

    bluetoothadapter *bt_adapter = new bluetoothadapter(adpater_data);

    if(bt_adapter != nullptr)
    {
        m_bluetooth_adapter_list.append(bt_adapter);
        m_bluetooth_adapter_name_list.append(dev_name);
        KyInfo() << "dev_defaultAdapterMark:" << dev_defaultAdapterMark
                    << "m_bluetooth_adapter_list:"  << m_bluetooth_adapter_list
                    << "m_bluetooth_adapter_name_list: " << m_bluetooth_adapter_name_list
                    << "m_bluetooth_adapter_address_list: " << m_bluetooth_adapter_address_list ;

        if(dev_defaultAdapterMark)//是默认适配器，读取设备列表
        {
            m_default_bluetooth_adapter = bt_adapter;
            bindDefaultAdapterReportData();
            getDefaultAdapterDevices();
        }
        return 0;
    }
    KyInfo() << address << ":data read fail! ";

    return 1;
}


QMap<QString, QVariant> BlueToothDBusService::getAdapterAttr(QString adapterAddr , QString datakey)
{
    QDBusMessage message = QDBusMessage::createMethodCall(SERVICE,
                                                          PATH,
                                                          INTERFACE,
                                                          "getAdapterAttr");
    message << adapterAddr << datakey;
    QDBusReply<QMap<QString, QVariant>> reply_res = QDBusConnection::systemBus().call(message);
    return reply_res;
}

QStringList BlueToothDBusService::getDefaultAdapterPairedDev()
{
    QDBusMessage message = QDBusMessage::createMethodCall(SERVICE,
                                                          PATH,
                                                          INTERFACE,
                                                          "getDefaultAdapterPairedDev");
    QDBusReply<QStringList> reply_res = QDBusConnection::systemBus().call(message);
    return reply_res;
}

QStringList BlueToothDBusService::getDefaultAdapterAllDev()
{
    QDBusMessage message = QDBusMessage::createMethodCall(SERVICE,
                                                          PATH,
                                                          INTERFACE,
                                                          "getDefaultAdapterAllDev");
    QDBusReply<QStringList> reply_res = QDBusConnection::systemBus().call(message);
    return reply_res;
}

void BlueToothDBusService::bluetoothDeviceDataAnalysis(QMap<QString ,QVariant> value,
                                 QString         &dev_address             ,
                                 QString         &dev_name                ,
                                 QString         &dev_showName            ,
                                 bluetoothdevice::DEVICE_TYPE     &dev_type ,
                                 bool            &dev_paired              ,
                                 bool            &dev_trusted             ,
                                 bool            &dev_blocked             ,
                                 bool            &dev_connected           ,
                                 bool            &dev_pairing             ,
                                 bool            &dev_connecting          ,
                                 int             &dev_battery             ,
                                 int             &dev_connectFailedId     ,
                                 QString         &dev_connectFailedDisc   ,
                                 qint16          &dev_rssi                ,
                                 bool            &dev_sendFileMark        ,
                                 QString         &adapter_addr   )
{
    KyDebug() << value ;
    //解析bluetooth device数据
    QString key = "Paired";
    if(value.contains(key))
        dev_paired = value[key].toBool();

    key = "Trusted";
    if(value.contains(key))
        dev_trusted = value[key].toBool();

    key = "Blocked";
    if(value.contains(key))
        dev_blocked = value[key].toBool();

    key = "Connected";
    if(value.contains(key))
        dev_connected = value[key].toBool();

    key = "Name";
    if(value.contains(key))
        dev_name = value[key].toString();

    key = "ShowName";
    if(value.contains(key))
        dev_showName = value[key].toString();

    key = "Addr";
    if(value.contains(key))
        dev_address = value[key].toString();

    key = "Type";
    if(value.contains(key))
        dev_type = bluetoothdevice::DEVICE_TYPE(value[key].toInt());

    key = "Pairing";
    if(value.contains(key))
        dev_pairing = value[key].toBool();

    key = "Connecting";
    if(value.contains(key))
        dev_connecting = value[key].toBool();

    key = "Battery";
    if(value.contains(key))
        dev_battery = value[key].toInt();

    key = "ConnectFailedId";
    if(value.contains(key))
        dev_connectFailedId = value[key].toInt();

    key = "ConnectFailedDisc";
    if(value.contains(key))
        dev_connectFailedDisc = value[key].toString();

    key = "Rssi";
    if(value.contains(key))
        dev_rssi = value[key].toInt();

    key = "FileTransportSupport";
    if(value.contains(key))
        dev_sendFileMark = value[key].toBool();

    key = "Adapter";
    if(value.contains(key))
        adapter_addr = value[key].toString();

}

bluetoothdevice * BlueToothDBusService::createOneBleutoothDeviceForAddress(QString address)
{
    KyDebug() ;
    bluetoothdevice * dev = nullptr;
    QString     device_addr = address,
                adapter_addr,
                dev_name,
                dev_showName,
                dev_connectFailedDisc;
    bluetoothdevice::DEVICE_TYPE     dev_type;
    bool        dev_paired              ,
                dev_trusted             ,
                dev_blocked             ,
                dev_connected           ,
                dev_pairing             ,
                dev_connecting          ,
                dev_sendFileMark        ;
    int         dev_battery             ,
                dev_connectFailedId     ;
    qint16      dev_rssi                ;

    QMap<QString,QVariant> value =  getDevAttr(device_addr);
    bluetoothDeviceDataAnalysis(value,
                                device_addr             ,
                                dev_name                ,
                                dev_showName            ,
                                dev_type                ,
                                dev_paired              ,
                                dev_trusted             ,
                                dev_blocked             ,
                                dev_connected           ,
                                dev_pairing             ,
                                dev_connecting          ,
                                dev_battery             ,
                                dev_connectFailedId     ,
                                dev_connectFailedDisc   ,
                                dev_rssi                ,
                                dev_sendFileMark        ,
                                adapter_addr);

    KyDebug()  << "device_addr:" << device_addr ;
    KyDebug()  << "dev_name:" << dev_name ;
    KyDebug()  << "dev_showName:" << dev_showName ;
    KyDebug()  << "dev_type:" << dev_type ;
    KyDebug()  << "dev_paired:" << dev_paired ;
    KyDebug()  << "dev_trusted:" << dev_trusted ;
    KyDebug()  << "dev_blocked:" << dev_blocked ;
    KyDebug()  << "dev_connected:" << dev_connected ;
    KyDebug()  << "dev_pairing:" << dev_pairing ;
    KyDebug()  << "dev_connecting:" << dev_connecting ;
    KyDebug()  << "dev_battery:" << dev_battery ;
    KyDebug()  << "dev_connectFailedId:" << dev_connectFailedId ;
    KyDebug()  << "dev_connectFailedDisc:" << dev_connectFailedDisc ;
    KyDebug()  << "dev_rssi:" << dev_rssi ;
    KyDebug()  << "dev_sendFileMark:" << dev_sendFileMark ;
    KyDebug()  << "adapter_addr:" << adapter_addr ;


//    dev = new bluetoothdevice(device_addr             ,
//                              dev_name                ,
//                              dev_showName            ,
//                              dev_type                ,
//                              dev_paired              ,
//                              dev_trusted             ,
//                              dev_blocked             ,
//                              dev_connected           ,
//                              dev_pairing             ,
//                              dev_connecting          ,
//                              dev_battery             ,
//                              dev_connectFailedId     ,
//                              dev_connectFailedDisc   ,
//                              dev_rssi                ,
//                              dev_sendFileMark        ,
//                              adapter_addr
//                              );

    dev = new bluetoothdevice(value);

    if (dev == nullptr)
        return nullptr;

    return dev;
}

QMap<QString,QVariant> BlueToothDBusService::getDevAttr(QString dev_address)
{
    QDBusMessage message = QDBusMessage::createMethodCall(SERVICE,
                                                          PATH,
                                                          INTERFACE,
                                                          "getDevAttr");
    message << dev_address;
    QDBusReply<QMap<QString,QVariant>> reply_res = QDBusConnection::systemBus().call(message);
    return reply_res;
}


bool BlueToothDBusService::devRename(QString address,QString rename_str)
{
    KyInfo() << address << rename_str ;
    deviceDataAttr.remove("Name");
    deviceDataAttr.insert("Name", QVariant(rename_str));
    bool res = setDevAttr(address,deviceDataAttr);
    return res;
}


bool BlueToothDBusService::setDevTrusted(QString address,bool isTrusted)
{
    KyDebug()  << address << isTrusted ;
    deviceDataAttr.remove("Trusted");
    deviceDataAttr.insert("Trusted", QVariant(isTrusted));
    bool res = setDevAttr(address,deviceDataAttr);
    return res;
}

bool BlueToothDBusService::setDevAttr(QString addr,QMap<QString,QVariant> devAttr)
{
    KyWarning() << addr << devAttr;

    QDBusMessage dbusMsg = QDBusMessage::createMethodCall(SERVICE,
                                                          PATH,
                                                          INTERFACE,
                                                          "setDevAttr");
    dbusMsg << addr << devAttr;
    QDBusMessage response = QDBusConnection::systemBus().call(dbusMsg, QDBus::NoBlock);

    deviceDataAttr.clear();
    if ( response.type() == QDBusMessage::ReplyMessage)
    {
        return response.arguments().takeFirst().toBool();
    }
    else
        return false;
}

void BlueToothDBusService::bindServiceReportData()
{
    KyDebug()  ;
    QDBusConnection::systemBus().connect(SERVICE,
                                         PATH,
                                         INTERFACE,
                                         "updateClient",this, SLOT(reportUpdateClient()));


    QDBusConnection::systemBus().connect(SERVICE,
                                         PATH,
                                         INTERFACE,
                                         "adapterAddSignal",this, SLOT(reportAdapterAddSignal(QMap<QString ,QVariant>)));

    QDBusConnection::systemBus().connect(SERVICE,
                                         PATH,
                                         INTERFACE,
                                         "adapterAttrChanged",this, SLOT(reportAdapterAttrChanged( QString,QMap<QString ,QVariant>)));

    QDBusConnection::systemBus().connect(SERVICE,
                                         PATH,
                                         INTERFACE,
                                         "adapterRemoveSignal",this, SLOT(reportAdapterRemoveSignal(QString)));


    QDBusConnection::systemBus().connect(SERVICE,
                                         PATH,
                                         INTERFACE,
                                         "deviceAddSignal",this, SLOT(reportDeviceAddSignal(QMap<QString ,QVariant>)));

    QDBusConnection::systemBus().connect(SERVICE,
                                         PATH,
                                         INTERFACE,
                                         "deviceAttrChanged",this, SLOT(reportDeviceAttrChanged(QString,QMap<QString ,QVariant>)));


    QDBusConnection::systemBus().connect(SERVICE,
                                          PATH,
                                          INTERFACE,
                                          "deviceRemoveSignal",this, SLOT(reportDeviceRemoveSignal(QString,QMap<QString ,QVariant>)));


    QDBusConnection::systemBus().connect(SERVICE,
                                         PATH,
                                         INTERFACE,
                                         "clearBluetoothDev",this, SLOT(reportClearBluetoothDev(QStringList)));

}

void BlueToothDBusService::bindDefaultAdapterReportData()
{
    KyDebug();
    if (m_default_bluetooth_adapter)
    {
        m_default_bluetooth_adapter->disconnect(this);
        connect(m_default_bluetooth_adapter,SIGNAL(adapterNameChanged(QString)),this,SIGNAL(adapterNameChanged(QString)));
        connect(m_default_bluetooth_adapter,SIGNAL(adapterPoweredChanged(bool)),this,SIGNAL(adapterPoweredChanged(bool)));
        connect(m_default_bluetooth_adapter,SIGNAL(adapterTrayIconChanged(bool)),this,SIGNAL(adapterTrayIconChanged(bool)));
        connect(m_default_bluetooth_adapter,SIGNAL(adapterDiscoverableChanged(bool)),this,SIGNAL(adapterDiscoverableChanged(bool)));
        connect(m_default_bluetooth_adapter,SIGNAL(adapterDiscoveringChanged(bool)),this,SIGNAL(adapterDiscoveringChanged(bool)));
        connect(m_default_bluetooth_adapter,SIGNAL(adapterAutoConnStatuChanged(bool)),this,SIGNAL(adapterActiveConnectionChanged(bool)));
    }
}

void BlueToothDBusService::bluetoothAdapterDataAnalysis(QMap<QString ,QVariant> value,
                                               QString &dev_name,
                                               QString &dev_address,
                                               bool    &dev_block ,
                                               bool    &dev_power,
                                               bool    &dev_pairing     ,
                                               bool    &dev_pairable    ,
                                               bool    &dev_connecting  ,
                                               bool    &dev_discovering ,
                                               bool    &dev_discoverable,
                                               bool    &dev_activeConnection,
                                               bool    &dev_defaultAdapterMark,
                                               bool    &dev_trayShow)
{
    KyDebug()  ;
    //解析adpater数据
    if(value.contains("Name"))
        dev_name = value["Name"].toString();

    if(value.contains("Addr"))
        dev_address = value["Addr"].toString();

    if(value.contains("Block"))
        dev_block = value["Block"].toBool();

    if(value.contains("Powered"))
        dev_power = value["Powered"].toBool();

    if(value.contains("Pairing"))
        dev_pairing = value["Pairing"].toBool();

    if(value.contains("Pairable"))
        dev_pairable = value["Pairable"].toBool();

    if(value.contains("Connecting"))
        dev_connecting = value["Connecting"].toBool();

    if(value.contains("Discovering"))
        dev_discovering = value["Discovering"].toBool();

    if(value.contains("Discoverable"))
        dev_discoverable = value["Discoverable"].toBool();

    if(value.contains("ActiveConnection"))
        dev_activeConnection = value["ActiveConnection"].toBool();

    if(value.contains("DefaultAdapter"))
        dev_defaultAdapterMark = value["DefaultAdapter"].toBool();

    if(value.contains("trayShow"))
        dev_trayShow = value["trayShow"].toBool();

    KyDebug()  << "end" ;

}

void BlueToothDBusService::reportAdapterAddSignal(QMap<QString ,QVariant> value)
{
    KyDebug() << value;

    QString dev_name,
            dev_address;
    bool    dev_block ,
            dev_power,
            dev_pairing     ,
            dev_pairable    ,
            dev_connecting  ,
            dev_discovering ,
            dev_discoverable,
            dev_activeConnection,
            dev_defaultAdapterMark,
            dev_trayShow;

    bluetoothAdapterDataAnalysis(value,
                                 dev_name,
                                 dev_address,
                                 dev_block ,
                                 dev_power,
                                 dev_pairing     ,
                                 dev_pairable    ,
                                 dev_connecting  ,
                                 dev_discovering ,
                                 dev_discoverable,
                                 dev_activeConnection,
                                 dev_defaultAdapterMark,
                                 dev_trayShow);


    foreach (auto var, m_bluetooth_adapter_list)
    {
        if (dev_address == var->getDevAddress())
        {
            KyWarning() << "Adapter already exists:" << dev_name << dev_address;
            return;
        }
    }

//    bluetoothadapter *bt_adapter = new bluetoothadapter(dev_name        ,
//                                                        dev_address     ,
//                                                        dev_block       ,
//                                                        dev_power       ,
//                                                        dev_pairing     ,
//                                                        dev_pairable    ,
//                                                        dev_connecting  ,
//                                                        dev_discovering ,
//                                                        dev_discoverable,
//                                                        dev_activeConnection,
//                                                        dev_defaultAdapterMark ,
//                                                        dev_trayShow);

    bluetoothadapter *bt_adapter = new bluetoothadapter(value);

    if(bt_adapter != nullptr)
    {
        m_bluetooth_adapter_list.append(bt_adapter);
        m_bluetooth_adapter_address_list.append(dev_address);
        m_bluetooth_adapter_name_list.append(dev_name);
        KyInfo()  << "add an adapter (mac):"  << dev_address ;

//        KyWarning()  << "add an adapter (mac):"  << dev_address <<
//                        "dev_defaultAdapterMark:"  << dev_defaultAdapterMark <<
//                        "m_bluetooth_adapter_list:"  << m_bluetooth_adapter_list ;

        if(dev_defaultAdapterMark || 1 == m_bluetooth_adapter_list.size())//是默认适配器/只有一个适配器，读取设备列表
        {
            m_default_bluetooth_adapter = bt_adapter;
            //读取默认适配器下配对和扫描出的设备
            bindDefaultAdapterReportData();
            getDefaultAdapterDevices();

            if (m_bluetooth_adapter_list.size() > 1)
            {
                int indx = 0;
                if (m_bluetooth_adapter_list.size()-1 >= 0)
                    indx = m_bluetooth_adapter_list.size()-1 ;
                emit defaultAdapterChangedSignal(indx);
            }
        }
        emit adapterAddSignal(dev_name);
    }
}

void BlueToothDBusService::reportAdapterRemoveSignal(QString address)
{
    KyDebug() << address << m_bluetooth_adapter_address_list ;

    int indx = -1;
    foreach (auto  val, m_bluetooth_adapter_list) {
        KyInfo() << " val->getDevAddress():" <<  val->getDevAddress();
        indx++;
        if (address == val->getDevAddress())
            break;
    }
    if (-1 == indx)
    {
        KyInfo() << "-1" << "m_bluetooth_adapter_list:" << m_bluetooth_adapter_list << m_bluetooth_adapter_list.size();
        return;
    }
    KyDebug() << "indx:" <<indx <<
               "m_bluetooth_adapter_list" << m_bluetooth_adapter_list <<
               "m_bluetooth_adapter_name_list:" << m_bluetooth_adapter_name_list <<
               "m_bluetooth_adapter_address_list" << m_bluetooth_adapter_address_list;

    if (nullptr != m_default_bluetooth_adapter)
    {
        KyDebug() << "m_default_bluetooth_adapter->getDevAddress():" << m_default_bluetooth_adapter->getDevAddress();
        if (address == m_default_bluetooth_adapter->getDevAddress())
        {
            //emit btServiceRestart();
            m_default_bluetooth_adapter = nullptr;
        }
    }

    if (indx < m_bluetooth_adapter_name_list.size())
        m_bluetooth_adapter_name_list.removeAt(indx);

    if (indx < m_bluetooth_adapter_address_list.size())
        m_bluetooth_adapter_address_list.removeAt(indx);

    if (indx < m_bluetooth_adapter_list.size())
    {
        bluetoothadapter * adapter_dev = m_bluetooth_adapter_list.at(indx);
        m_bluetooth_adapter_list.removeAt(indx);
        adapter_dev->disconnect();
        adapter_dev->deleteLater();
    }
    Q_EMIT adapterRemoveSignal(indx);
}

void BlueToothDBusService::serviceChangedDefaultAdapter(int indx)
{
    KyDebug() << "adapter address indx:" << indx ;
    if(m_default_bluetooth_adapter && m_default_bluetooth_adapter->getDevAddress() == m_bluetooth_adapter_list.at(indx)->getDevAddress())
    {
        KyDebug() << "default Adapter not Changed:" << indx ;
        return;
    }
    m_default_bluetooth_adapter =  m_bluetooth_adapter_list.at(indx);
    this->bindDefaultAdapterReportData();
    getDefaultAdapterDevices();

    Q_EMIT defaultAdapterChangedSignal(indx);
}

void BlueToothDBusService::reportAdapterAttrChanged(QString address,QMap<QString ,QVariant> value)
{
    KyDebug() << "adapter address:" << address << value;
//    KyWarning() << "adapter address:" << address << value;
    bool flag = false ;
    int indx = 0;
    for (bluetoothadapter * btAdapter : m_bluetooth_adapter_list) {
        if (address == btAdapter->getDevAddress())
        {
            flag = true;
            break;
        }
        indx++;
    }

    KyInfo() << "bt adapter list index:" << indx ;
    if (flag && indx < m_bluetooth_adapter_list.size())
    {
        QString key;
        key = "Name";
        if(value.contains(key) && (QVariant::String == value[key].type()))
        {
            QString new_name = value[key].toString();
            m_bluetooth_adapter_list.at(indx)->resetDeviceName(value[key].toString());
            if (m_bluetooth_adapter_name_list.count() > indx)
            {
                m_bluetooth_adapter_name_list.removeAt(indx);
                m_bluetooth_adapter_name_list.insert(indx,new_name);
            }
            Q_EMIT adapterNameChangedOfIndex(indx,new_name);
        }

        //key = "Addr";
        key = "Block";
        if(value.contains(key) && (QVariant::Bool == value[key].type()))
        {
//            bool dev_block = value[key].toBool();
        }

        key = "Powered";
        if(value.contains(key) && (QVariant::Bool == value[key].type()))
        {
//            bool dev_power = value[key].toBool();
            m_bluetooth_adapter_list.at(indx)->setAdapterPower(value[key].toBool());
        }

        key = "Pairing";
        if(value.contains(key) && (QVariant::Bool == value[key].type()))
        {
            //bool dev_pairing = value[key].toBool();
            m_bluetooth_adapter_list.at(indx)->setAdapterPairing(value[key].toBool());
        }

        key = "Pairable";
        if(value.contains("Pairable") && (QVariant::Bool == value[key].type()))
        {
//            bool dev_pairable = value["Pairable"].toBool();
            m_bluetooth_adapter_list.at(indx)->setAdapterPairable(value[key].toBool());

        }

        key = "Connecting";
        if(value.contains(key) && (QVariant::Bool == value[key].type()))
        {
//            bool dev_connecting = value[key].toBool();
            m_bluetooth_adapter_list.at(indx)->setAdapterConnecting(value[key].toBool());

        }

        key = "Discovering";
        if(value.contains(key) && (QVariant::Bool == value[key].type()))
        {
//            bool dev_discovering = value[key].toBool();
            m_bluetooth_adapter_list.at(indx)->setAdapterDiscovering(value[key].toBool());
        }

        key = "Discoverable";
        if(value.contains(key) && (QVariant::Bool == value[key].type()))
        {
//            bool dev_discoverable = value[key].toBool();
            m_bluetooth_adapter_list.at(indx)->setAdapterDiscoverable(value[key].toBool());
        }

        key = "ActiveConnection";
        if(value.contains(key) && (QVariant::Bool == value[key].type()))
        {
//            bool dev_activeConnection = value[key].toBool();
            m_bluetooth_adapter_list.at(indx)->setAdapterAutoConn(value[key].toBool());

        }

        key = "DefaultAdapter";
        if(value.contains(key) && (QVariant::Bool == value[key].type()))
        {
//            bool dev_defaultAdapterMark = value[key].toBool();
            m_bluetooth_adapter_list.at(indx)->setAdapterDefaultMark(value[key].toBool());
            if (value[key].toBool())
            {
                serviceChangedDefaultAdapter(indx);
            }
        }

//        support_a2dp_sink
//        support_a2dp_source
//        support_hfphsp_ag
//        support_hfphsp_hf
//        FileTransportSupport

        key = "trayShow";
        if(value.contains(key) && (QVariant::Bool == value[key].type()))
        {
//            bool dev_trayShow = value[key].toBool();
            m_bluetooth_adapter_list.at(indx)->setAdapterTrayShow(value[key].toBool());
        }

    }
    else
    {
        KyWarning()  << "There is no corresponding adapter!" ;
    }

}

void BlueToothDBusService::reportDeviceAddSignal(QMap<QString ,QVariant> value)
{
    KyDebug() << value;

    if(nullptr == m_default_bluetooth_adapter)
    {
        KyWarning() << "m_default_bluetooth_adapter is nullptr!" ;
        return;
    }

    bluetoothdevice * dev = nullptr;
    QString      dev_address             ,
                 adapter_addr            ,
                 dev_name                ,
                 dev_showName            ,
                 dev_connectFailedDisc   ;
    bluetoothdevice::DEVICE_TYPE
                 dev_type                ;
    bool         dev_paired              ,
                 dev_trusted             ,
                 dev_blocked             ,
                 dev_connected           ,
                 dev_pairing             ,
                 dev_connecting          ,
                 dev_sendFileMark        ;
    int          dev_battery             ,
                 dev_connectFailedId     ;
    qint16       dev_rssi                ;


    bluetoothDeviceDataAnalysis(value,
                                dev_address             ,
                                dev_name                ,
                                dev_showName            ,
                                dev_type                ,
                                dev_paired              ,
                                dev_trusted             ,
                                dev_blocked             ,
                                dev_connected           ,
                                dev_pairing             ,
                                dev_connecting          ,
                                dev_battery             ,
                                dev_connectFailedId     ,
                                dev_connectFailedDisc   ,
                                dev_rssi                ,
                                dev_sendFileMark        ,
                                adapter_addr            
                                );


    if(adapter_addr != m_default_bluetooth_adapter->getDevAddress())
    {
        KyWarning() << "m_default_bluetooth_adapter != device default adapter addr" <<
                       adapter_addr <<
                       m_default_bluetooth_adapter->getDevAddress();
        return ;
    }

//    KyDebug()  << "dev_address:" << dev_address ;
//    KyDebug()  << "dev_name:" << dev_name ;
//    KyDebug()  << "dev_showName:" << dev_showName ;
//    KyDebug()  << "dev_type:" << dev_type ;
//    KyDebug()  << "dev_paired:" << dev_paired ;
//    KyDebug()  << "dev_trusted:" << dev_trusted ;
//    KyDebug()  << "dev_blocked:" << dev_blocked ;
//    KyDebug()  << "dev_connected:" << dev_connected ;
//    KyDebug()  << "dev_pairing:" << dev_pairing ;
//    KyDebug()  << "dev_connecting:" << dev_connecting ;
//    KyDebug()  << "dev_battery:" << dev_battery ;
//    KyDebug()  << "dev_connectFailedId:" << dev_connectFailedId ;
//    KyDebug()  << "dev_connectFailedDisc:" << dev_connectFailedDisc ;
//    KyDebug()  << "dev_rssi:" << dev_rssi ;
//    KyDebug()  << "dev_sendFileMark:" << dev_sendFileMark ;
//    KyDebug()  << "adapter_addr:" << adapter_addr ;

//    dev = new bluetoothdevice(dev_address             ,
//                              dev_name                ,
//                              dev_showName            ,
//                              dev_type                ,
//                              dev_paired              ,
//                              dev_trusted             ,
//                              dev_blocked             ,
//                              dev_connected           ,
//                              dev_pairing             ,
//                              dev_connecting          ,
//                              dev_battery             ,
//                              dev_connectFailedId     ,
//                              dev_connectFailedDisc   ,
//                              dev_rssi                ,
//                              dev_sendFileMark        ,
//                              adapter_addr
//                              );

    dev = new bluetoothdevice(value);

    if (nullptr != dev && !m_default_bluetooth_adapter->m_bt_dev_list.contains(dev->getDevAddress()))
    {
        m_default_bluetooth_adapter->m_bt_dev_list.insert(dev->getDevAddress(), dev);
        emit deviceAddSignal(dev->getDevAddress());
    }
    else if (m_default_bluetooth_adapter->m_bt_dev_list.contains(dev->getDevAddress()))
    {
        //exist : update 
        reportDeviceAttrChanged(dev->getDevAddress(),value);
        //界面add
        emit deviceAddSignal(dev->getDevAddress());
    }
    else
    {
        dev->deleteLater();
    }
}

void BlueToothDBusService::reportDeviceAttrChanged(QString address,QMap<QString ,QVariant> value)
{
    KyDebug() <<  "device address:" << address << value;
    if (nullptr == m_default_bluetooth_adapter)
    {
        KyWarning() <<  "m_default_bluetooth_adapter is nullptr!";
        return;
    }

    if (m_default_bluetooth_adapter->m_bt_dev_list.contains(address))
    {
        QString key;
        key = "Paired";
        if(value.contains(key) && (QVariant::Bool == value[key].type()))
        {
            m_default_bluetooth_adapter->m_bt_dev_list[address]->devPairedChanged(value[key].toBool());
            if (value[key].toBool())
                Q_EMIT devicePairedSuccess(address);
        }

        key = "Trusted";
        if(value.contains(key) && (QVariant::Bool == value[key].type()))
        {
            m_default_bluetooth_adapter->m_bt_dev_list[address]->setDevTrust(value[key].toBool());
        }

//        key = "Blocked";
//        if(value.contains(key) && (QVariant::Bool == value[key].type()))
//        {
//            bool dev_blocked = value[key].toBool();
//        }

        key = "Connected";
        if(value.contains(key) && (QVariant::Bool == value[key].type()))
        {
            m_default_bluetooth_adapter->m_bt_dev_list[address]->devConnectedChanged(value[key].toBool());
        }

        key = "Name";
        if(value.contains(key) && (QVariant::String == value[key].type()))
        {
            m_default_bluetooth_adapter->m_bt_dev_list[address]->resetDeviceName(value[key].toString());
        }

        key = "ShowName";
        if(value.contains(key) && (QVariant::String == value[key].type()))
        {
            m_default_bluetooth_adapter->m_bt_dev_list[address]->setDevShowName(value[key].toString());
        }

        key = "Type";
        if(value.contains(key) && (QVariant::Int == value[key].type()))
        {
            bluetoothdevice::DEVICE_TYPE dev_type = bluetoothdevice::DEVICE_TYPE(value[key].toInt());
            m_default_bluetooth_adapter->m_bt_dev_list[address]->setDevType(dev_type);
        }

        key = "Pairing";
        if(value.contains(key) && (QVariant::Bool == value[key].type()))
        {
            m_default_bluetooth_adapter->m_bt_dev_list[address]->setDevPairing(value[key].toBool());
        }

        key = "Connecting";
        if(value.contains(key) && (QVariant::Bool == value[key].type()))
        {
            m_default_bluetooth_adapter->m_bt_dev_list[address]->setDevConnecting(value[key].toBool());
        }

//        key = "Battery";
//        if(value.contains(key) && (QVariant::Int == value[key].type()))
//        {
//            int dev_battery = value[key].toInt();
//        }

        key = "ConnectFailedId";
        int dev_connectFailedId = 0;
        if(value.contains(key) && (QVariant::Int == value[key].type()))
        {
            dev_connectFailedId = value[key].toInt();
        }

        key = "ConnectFailedDisc";
        if(value.contains(key) && (QVariant::String == value[key].type()))
        {
            m_default_bluetooth_adapter->m_bt_dev_list[address]->setDevConnFailedInfo(dev_connectFailedId,value[key].toString());
        }

        key = "Rssi";
        if(value.contains(key) && (QVariant::Int == value[key].type()))
        {
            m_default_bluetooth_adapter->m_bt_dev_list[address]->setDevRssi(value[key].toInt());
        }

        key = "FileTransportSupport";
        if(value.contains(key) && (QVariant::Bool == value[key].type()))
        {
            m_default_bluetooth_adapter->m_bt_dev_list[address]->setDevSendFileMark(value[key].toInt());
        }
    }
    else
    {
        KyDebug() << address << "not add this dev";
        bluetoothdevice * dev = createOneBleutoothDeviceForAddress(address);
        if (dev)
        {
            m_default_bluetooth_adapter->m_bt_dev_list.insert(address,dev);
            Q_EMIT deviceAddSignal(address);
        }
    }

    return ;


#if 0
    bool flag = false ;
    int indx = 0;
    for (bluetoothdevice * btDevice : m_default_bluetooth_adapter->m_bluetooth_device_list) {
        if (address == btDevice->getDevAddress())
        {
            flag = true;
            break;
        }
        indx++;
    }

    KyDebug()<< "bt adapter list index:" << indx
            << "flag" << flag
            << "m_bluetooth_device_list size:" << m_default_bluetooth_adapter->m_bluetooth_device_list.size();

    if (flag && indx < m_default_bluetooth_adapter->m_bluetooth_device_list.size())
    {
        KyDebug()  << "test dev" <<
                   m_default_bluetooth_adapter->m_bluetooth_device_list.at(indx)->getDevAddress() <<
                   m_default_bluetooth_adapter->m_bluetooth_device_list.at(indx)->getDevName();

        QString key;
        key = "Paired";
        if(value.contains(key) && (QVariant::Bool == value[key].type()))
        {
//            bool dev_paired = value[key].toBool();
            m_default_bluetooth_adapter->m_bluetooth_device_list.at(indx)->devPairedChanged(value[key].toBool());
        }

        key = "Trusted";
        if(value.contains(key) && (QVariant::Bool == value[key].type()))
        {
//            bool dev_trusted = value[key].toBool();
            m_default_bluetooth_adapter->m_bluetooth_device_list.at(indx)->setDevTrust(value[key].toBool());
        }

//        key = "Blocked";
//        if(value.contains(key) && (QVariant::Bool == value[key].type()))
//        {
//            bool dev_blocked = value[key].toBool();
//        }


        key = "Connected";
        if(value.contains(key) && (QVariant::Bool == value[key].type()))
        {
//            bool dev_connected = value[key].toBool();
            m_default_bluetooth_adapter->m_bluetooth_device_list.at(indx)->devConnectedChanged(value[key].toBool());
        }

        key = "Name";
        if(value.contains(key) && (QVariant::String == value[key].type()))
        {
//            QString dev_name = value[key].toString();
            m_default_bluetooth_adapter->m_bluetooth_device_list.at(indx)->resetDeviceName(value[key].toString());
        }

        key = "ShowName";
        if(value.contains(key) && (QVariant::String == value[key].type()))
        {
            //            QString dev_name = value[key].toString();
            m_default_bluetooth_adapter->m_bluetooth_device_list.at(indx)->setDevShowName(value[key].toString());
        }

        key = "Type";
        if(value.contains(key) && (QVariant::Int == value[key].type()))
        {
            bluetoothdevice::DEVICE_TYPE dev_type = bluetoothdevice::DEVICE_TYPE(value[key].toInt());
            m_default_bluetooth_adapter->m_bluetooth_device_list.at(indx)->setDevType(dev_type);
        }

        key = "Pairing";
        if(value.contains(key) && (QVariant::Bool == value[key].type()))
        {
//            bool dev_pairing = value[key].toBool();
            m_default_bluetooth_adapter->m_bluetooth_device_list.at(indx)->setDevPairing(value[key].toBool());
        }

        key = "Connecting";
        if(value.contains(key) && (QVariant::Bool == value[key].type()))
        {
//            bool dev_connecting = value[key].toBool();
            m_default_bluetooth_adapter->m_bluetooth_device_list.at(indx)->setDevConnecting(value[key].toBool());
        }

//        key = "Battery";
//        if(value.contains(key) && (QVariant::Int == value[key].type()))
//        {
//            int dev_battery = value[key].toInt();
//        }

        key = "ConnectFailedId";
        int dev_connectFailedId = 0;
        if(value.contains(key) && (QVariant::Int == value[key].type()))
        {
            dev_connectFailedId = value[key].toInt();
        }

        key = "ConnectFailedDisc";
        if(value.contains(key) && (QVariant::String == value[key].type()))
        {
//           QString dev_connectFailedDisc = value[key].toString();
            m_default_bluetooth_adapter->m_bluetooth_device_list.at(indx)->setDevConnFailedInfo(dev_connectFailedId,value[key].toString());
        }

        key = "Rssi";
        if(value.contains(key) && (QVariant::Int == value[key].type()))
        {
//            int dev_rssi = value[key].toInt();
            m_default_bluetooth_adapter->m_bluetooth_device_list.at(indx)->setDevRssi(value[key].toInt());
        }

        key = "FileTransportSupport";
        if(value.contains(key) && (QVariant::Bool == value[key].type()))
        {
//            bool dev_sendFileMark = value[key].toBool();
            m_default_bluetooth_adapter->m_bluetooth_device_list.at(indx)->setDevSendFileMark(value[key].toInt());
        }
    }
    else
    {

    }
#endif
}


//int BlueToothDBusService::getDevListIndex(QString address)
//{
//    if (nullptr == m_default_bluetooth_adapter)
//    {
//        KyWarning() << "m_default_bluetooth_adapter is nullptr!" ;
//        return -1;
//    }
//    int indx = -1;
//    for (bluetoothdevice * btDevice : m_default_bluetooth_adapter->m_bluetooth_device_list)
//    {
//        indx++;
//        if (address == btDevice->getDevAddress())
//        {
//            break;
//        }
//    }
//    return indx;
//}

int BlueToothDBusService::reportDeviceRemoveSignal(QString devAddr,QMap<QString ,QVariant> value)
{
    KyDebug() << devAddr << value;

    if (nullptr == m_default_bluetooth_adapter)
    {
        KyDebug() << "m_default_bluetooth_adapter is nullptr!";
        return 1;
    }

    QString dev_adapter_addr ;
    QString key = "Adapter";

    if(value.contains(key) && (QVariant::String == value[key].type()))
        dev_adapter_addr = value[key].toString();
    if (dev_adapter_addr != m_default_bluetooth_adapter->getDevAddress())
    {
        KyDebug() << "dev_adapter_addr:" << dev_adapter_addr <<
                       "m_default_bluetooth_adapter->getDevAddress:" << m_default_bluetooth_adapter->getDevAddress();
        return 1;
    }

    if (!m_default_bluetooth_adapter->m_bt_dev_list.contains(devAddr))
    {
        KyDebug() << devAddr << "remove dev not exist";
        return 1;
    }

    if (m_default_bluetooth_adapter->m_bt_dev_list[devAddr]->getDevConnecting())
    {
        KyDebug() << devAddr << "Device is connecting!";
        return 1;
    }

    emit deviceRemoveSignal(devAddr);

    if (m_remainder_loaded_bluetooth_device_address_list.contains(devAddr))
    {
        m_remainder_loaded_bluetooth_device_address_list.removeAll(devAddr);
    }

    bluetoothdevice * dev = m_default_bluetooth_adapter->m_bt_dev_list[devAddr];
//    if (dev->isPaired())
//        m_default_bluetooth_adapter->m_bt_dev_paired_list.remove(devAddr);
    m_default_bluetooth_adapter->m_bt_dev_list.remove(devAddr);
    dev->disconnect();
    KyDebug() << "delete dev:" << dev->getDevName() << dev->getDevAddress();
    dev->deleteLater();

    return 0;
}

void BlueToothDBusService::reportClearBluetoothDev(QStringList addrList)
{
    KyDebug()<< addrList ;
    if (nullptr == m_default_bluetooth_adapter)
    {
        KyWarning() << "m_default_bluetooth_adapter is nullptr!";
        return;
    }

    for (QString addr : addrList) {
        //发送给界面移除信号
//        emit deviceRemoveSignal(addr);
        //移除列表中的数据
        QMap<QString, QVariant> value;
        value["Adapter"] = m_default_bluetooth_adapter->getDevAddress();
        if (0 == reportDeviceRemoveSignal(addr,value))
        {    //下发移除信号给服务
            devRemove(addr);
        }
    }
}

int BlueToothDBusService::devConnect(QString address)
{
    KyInfo()  << address ;
    int reply_res = 0;
    QDBusMessage mesg = QDBusMessage::createMethodCall(SERVICE,
                                                    PATH,
                                                    INTERFACE,
                                                    "devConnect");
    mesg << address;
    //KyDebug()  << m.arguments().at(0).value<QString>() <<;
    // 发送Message
    QDBusMessage res =  QDBusConnection::systemBus().call(mesg, QDBus::NoBlock);

    if(res.type() == QDBusMessage::ReplyMessage)
    {
       if(res.arguments().size() > 0)
       {
           reply_res = res.arguments().takeFirst().toInt();
           KyInfo() << reply_res;
       }
    }
    else
    {
       KyWarning()<< res.errorName() << ": "<< res.errorMessage();
    }

    return reply_res;
}

int BlueToothDBusService::devDisconnect(QString address)
{
    KyDebug();
    int reply_res = 0;

    QDBusMessage m = QDBusMessage::createMethodCall(SERVICE,
                                                    PATH,
                                                    INTERFACE,
                                                    "devDisconnect");
    m << address;
    KyDebug()  << m.arguments().at(0).value<QString>();
    // 发送Message
    QDBusMessage res = QDBusConnection::systemBus().call(m, QDBus::NoBlock);

    if(res.type() == QDBusMessage::ReplyMessage)
    {
       if(res.arguments().size() > 0)
       {
           reply_res = res.arguments().takeFirst().toInt();
           KyInfo() << reply_res;
       }
    }
    else
    {
       KyWarning()<< res.errorName() << ": "<< res.errorMessage();
    }

    return reply_res;

}

int BlueToothDBusService::devRemove(QString address)
{
    KyDebug() << address ;
    QStringList dev_addrList;
    dev_addrList.clear();
    dev_addrList.append(address);
    int reply_res = devRemove(dev_addrList);
    return reply_res;
}

int BlueToothDBusService::devRemove(QStringList addressList)
{
    KyDebug()  << addressList ;
    int reply_res = 0;

    QDBusMessage m = QDBusMessage::createMethodCall(SERVICE,
                                                    PATH,
                                                    INTERFACE,
                                                    "devRemove");
    m << addressList;
    KyDebug()  << m.arguments().at(0).value<QStringList>();
    // 发送Message
    QDBusMessage res = QDBusConnection::systemBus().call(m, QDBus::NoBlock);

    if(res.type() == QDBusMessage::ReplyMessage)
    {
       if(res.arguments().size() > 0)
       {
           reply_res = res.arguments().takeFirst().toInt();
           KyInfo() << reply_res;
       }
    }
    else
    {
       KyDebug()<< res.errorName() << ": "<< res.errorMessage();
    }

    return reply_res;
}


int BlueToothDBusService::sendFiles(QString address)
{
    KyDebug()  ;
    int reply_res = 0;

    QDBusMessage m = QDBusMessage::createMethodCall(SERVICE,
                                                    PATH,
                                                    INTERFACE,
                                                    "setSendTransferDeviceMesg");
    m << address;
    KyDebug()  << m.arguments().at(0).value<QString>();
    //使用sessionBus 发送Message
    QDBusMessage res = QDBusConnection::sessionBus().call(m, QDBus::NoBlock);
    if(res.type() == QDBusMessage::ReplyMessage)
    {
       if(res.arguments().size() > 0)
       {
           reply_res = res.arguments().takeFirst().toInt();
           KyInfo() << reply_res;
       }
    }
    else
    {
       KyWarning()<< res.errorName() << ": "<< res.errorMessage();
    }

    return reply_res;
}

bool BlueToothDBusService::setDefaultAdapterAttr(QMap<QString, QVariant> adpAttr)
{
    KyDebug() << adpAttr;

    QDBusMessage dbusMsg = QDBusMessage::createMethodCall(SERVICE,
                                                          PATH,
                                                          INTERFACE,
                                                          "setDefaultAdapterAttr");
    dbusMsg << adpAttr;
    QDBusMessage response = QDBusConnection::systemBus().call(dbusMsg, QDBus::NoBlock);

    defaultAdapterDataAttr.clear();

    if (response.type() == QDBusMessage::ReplyMessage)
    {
        return response.arguments().takeFirst().toBool();
    } else
        return false;
}

void BlueToothDBusService::setDefaultAdapterSwitchStatus(bool power)
{
    KyDebug() << power;
//    KyWarning() << "===============" <<  power;
    defaultAdapterDataAttr.remove("Powered");
    defaultAdapterDataAttr.insert("Powered", QVariant(power));
    setDefaultAdapterAttr(defaultAdapterDataAttr);
}

void BlueToothDBusService::setDefaultAdapterDiscoverableStatus(bool status)
{
    KyDebug() <<  status;
//    KyWarning() << "===============" <<  status;
    defaultAdapterDataAttr.remove("Discoverable");
    defaultAdapterDataAttr.insert("Discoverable", QVariant(status));
    setDefaultAdapterAttr(defaultAdapterDataAttr);
}

void BlueToothDBusService::setDefaultAdapterScanOn(bool scanStatus)
{
    KyDebug() << "set adapter state:" << scanStatus ;
//    KyWarning() << "===============" <<  scanStatus;

    defaultAdapterDataAttr.remove("Discovering");
    defaultAdapterDataAttr.insert("Discovering", QVariant(scanStatus));
    setDefaultAdapterAttr(defaultAdapterDataAttr);
}

void BlueToothDBusService::setDefaultAdapterName(QString name)
{
    KyDebug() << name ;

    defaultAdapterDataAttr.remove("Name");
    defaultAdapterDataAttr.insert("Name", QVariant(name));
    setDefaultAdapterAttr(defaultAdapterDataAttr);
}

void BlueToothDBusService::setAutoConnectAudioDevStatus(bool status)
{
    KyDebug() << status ;
    defaultAdapterDataAttr.remove("ActiveConnection");
    defaultAdapterDataAttr.insert("ActiveConnection", QVariant(status));
    setDefaultAdapterAttr(defaultAdapterDataAttr);
}

void BlueToothDBusService::setTrayIconShowStatus(bool status)
{
    KyDebug() << status ;
    defaultAdapterDataAttr.remove("trayShow");
    defaultAdapterDataAttr.insert("trayShow", QVariant(status));
    setDefaultAdapterAttr(defaultAdapterDataAttr);
}

int BlueToothDBusService::setDefaultAdapter(QString address)
{
    KyInfo() << address ;
    QDBusMessage dbusMsg = QDBusMessage::createMethodCall(SERVICE,
                                                          PATH,
                                                          INTERFACE,
                                                          "setDefaultAdapter");
    dbusMsg << address;
    QDBusMessage response = QDBusConnection::systemBus().call(dbusMsg, QDBus::NoBlock);

    if (response.type() == QDBusMessage::ReplyMessage)
    {
        return response.arguments().takeFirst().toInt();
    } else
        return (-1);
}

bool BlueToothDBusService::getTransferInfo(QString dev) {
    QDBusMessage m = QDBusMessage::createMethodCall(SERVICE,
                                                    PATH,
                                                    INTERFACE,
                                                    "getTransferDevAndStatus");
    m << dev;
    QDBusMessage response = QDBusConnection::systemBus().call(m);
    if (response.type() == QDBusMessage::ReplyMessage)
    {
        //KyDebug() << "<<______>>";
        return response.arguments().takeFirst().toBool();
    }
    //默认暴露文件发送接口
    return true;
}

void BlueToothDBusService::devLoadingTimeoutSlot()
{
    KyDebug() << m_remainder_loaded_bluetooth_device_address_list;

    if (m_remainder_loaded_bluetooth_device_address_list.size() > 0)
    {
        m_loading_dev_timer->stop();
        QString str_addr = m_remainder_loaded_bluetooth_device_address_list.at(0);

        bluetoothdevice * dev = createOneBleutoothDeviceForAddress(str_addr);
        if (nullptr != dev)
        {
            m_remainder_loaded_bluetooth_device_address_list.pop_front();
            m_default_bluetooth_adapter->m_bt_dev_list[str_addr] = dev;
            emit deviceAddSignal(str_addr);
        }
        else
        {
            KyWarning() << str_addr << " get not data!";
        }
        if (m_remainder_loaded_bluetooth_device_address_list.size() > 0)
        {
            m_loading_dev_timer->start();
        }
    }
    else
    {
        m_loading_dev_timer->stop();
    }

}
