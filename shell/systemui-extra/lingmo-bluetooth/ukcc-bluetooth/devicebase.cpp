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

#include "devicebase.h"

//devicebase
devicebase::devicebase()
{

}
//devicebase end

//bluetoothadapter
bluetoothadapter::bluetoothadapter(QString dev_name        ,
                                   const QString dev_address     ,
                                   bool    dev_block       ,
                                   bool    dev_power       ,
                                   bool    dev_pairing     ,
                                   bool    dev_pairable    ,
                                   bool    dev_connecting  ,
                                   bool    dev_discovering ,
                                   bool    dev_discoverable,
                                   bool    dev_activeConnection,
                                   bool    dev_defaultAdapterMark,
                                   bool    dev_trayShow)
    :m_dev_name(dev_name),
     m_dev_address(dev_address)          ,
     m_dev_block(dev_block)              ,
     m_dev_power(dev_power)              ,
     m_dev_pairing(dev_pairing)          ,
     m_dev_pairable(dev_pairable)        ,
     m_dev_connecting(dev_connecting)    ,
     m_dev_discovering(dev_discovering)  ,
     m_dev_discoverable(dev_discoverable),
     m_dev_activeConnection(dev_activeConnection),
     m_dev_defaultAdapterMark(dev_defaultAdapterMark),
     m_dev_trayShow(dev_trayShow)
{
    KyDebug();
    this->setObjectName(dev_address);
}

bluetoothadapter::bluetoothadapter(QMap<QString ,QVariant> value)
{
    bluetoothAdapterDataAnalysis(value);
}

void bluetoothadapter::bluetoothAdapterDataAnalysis(QMap<QString ,QVariant> value)
{
    //解析adpater数据
    QString key = "Name";
    if(value.contains(key))
        this->m_dev_name = value[key].toString();

    key = "Addr";
    if(value.contains(key))
        this->m_dev_address = value[key].toString();

    key = "Block";
    if(value.contains(key))
        this->m_dev_block = value[key].toBool();

    key = "Powered";
    if(value.contains(key))
        this->m_dev_power = value[key].toBool();

    key = "Pairing";
    if(value.contains(key))
        this->m_dev_pairing = value[key].toBool();

    key = "Pairable";
    if(value.contains(key))
        this->m_dev_pairable = value[key].toBool();

    key = "Connecting";
    if(value.contains(key))
        this->m_dev_connecting = value[key].toBool();

    key = "Discovering";
    if(value.contains(key))
        this->m_dev_discovering = value[key].toBool();

    key = "Discoverable";
    if(value.contains(key))
        this->m_dev_discoverable = value[key].toBool();

    key = "ActiveConnection";
    if(value.contains(key))
        this->m_dev_activeConnection = value[key].toBool();

    key = "DefaultAdapter";
    if(value.contains(key))
        this->m_dev_defaultAdapterMark = value[key].toBool();

    key = "trayShow";
    if(value.contains(key))
        this->m_dev_trayShow = value[key].toBool();
}

void bluetoothadapter::resetDeviceName(QString new_dev_name)
{
    KyDebug();

    if (new_dev_name != this->m_dev_name)
    {
        this->m_dev_name = new_dev_name;
        Q_EMIT adapterNameChanged(this->m_dev_name);
    }
}

QString bluetoothadapter::getDevName()
{
    KyDebug();
    return this->m_dev_name;
}

QString bluetoothadapter::getDevAddress()
{
    KyDebug();
    return this->m_dev_address;
}


void bluetoothadapter::setAdapterPower(bool value)
{
    KyDebug() << "this->m_dev_power:" << this->m_dev_power << " value:" << value;

    if (value != this->m_dev_power)
    {
        this->m_dev_power = value;
        Q_EMIT adapterPoweredChanged(this->m_dev_power);
    }
}

bool bluetoothadapter::getAdapterPower()
{
    KyDebug();
    return this->m_dev_power;
}


void bluetoothadapter::setAdapterPairing(bool value)
{
    KyDebug() << value ;
    if (value != this->m_dev_pairing)
    {
        this->m_dev_pairing = value;
        Q_EMIT adapterPairingChanged(this->m_dev_pairing);
    }
}
bool bluetoothadapter::getAdapterPairing()
{
    return this->m_dev_pairing;
}

void bluetoothadapter::setAdapterConnecting(bool value)
{
    KyDebug() << value ;
    if (value != this->m_dev_connecting)
    {
        this->m_dev_connecting = value;
        Q_EMIT adapterConnectingChanged(this->m_dev_connecting);
    }
}

bool bluetoothadapter::getAdapterConnecting()
{
    return this->m_dev_connecting;
}

void bluetoothadapter::setAdapterPairable(bool value)
{
    if (value != this->m_dev_pairable)
    {
        this->m_dev_pairable = value;
        Q_EMIT adapterPairableChanged(this->m_dev_pairable);
    }
}

bool bluetoothadapter::getAdapterPairable()
{
    return this->m_dev_pairable;

}

void bluetoothadapter::setAdapterDiscovering(bool value)
{
    KyDebug() << value ;
    if (value != this->m_dev_discovering)
    {
        this->m_dev_discovering = value;
        Q_EMIT adapterDiscoveringChanged(this->m_dev_discovering);
    }
}

bool bluetoothadapter::getAdapterDiscovering()
{
    KyDebug() ;
    return this->m_dev_discovering;
}

void bluetoothadapter::setAdapterDiscoverable(bool value)
{
    KyDebug() ;
    if (value != this->m_dev_discoverable)
    {
        this->m_dev_discoverable = value;
        Q_EMIT adapterDiscoverableChanged(this->m_dev_discoverable);
    }
}

bool bluetoothadapter::getAdapterDiscoverable()
{
    KyDebug();
    return this->m_dev_discoverable;
}


void bluetoothadapter::setAdapterAutoConn(bool value)
{
    KyDebug();
    if (value != this->m_dev_activeConnection)
    {
        this->m_dev_activeConnection = value;
        Q_EMIT adapterAutoConnStatuChanged(this->m_dev_activeConnection);
    }
}
bool bluetoothadapter::getAdapterAutoConn()
{
    return  this->m_dev_activeConnection;
}

void bluetoothadapter::setAdapterDefaultMark(bool mark)
{
    KyDebug();
    if (mark != this->m_dev_defaultAdapterMark)
    {
        this->m_dev_defaultAdapterMark = mark;
        if (mark)
            Q_EMIT defaultAdapterChanged(this->m_dev_address);
    }
}

bool bluetoothadapter::getAdapterDefaultMark()
{
    KyDebug();
    return this->m_dev_defaultAdapterMark;
}

void bluetoothadapter::setAdapterTrayShow(bool trayShow)
{
    KyDebug();
    if (trayShow != this->m_dev_trayShow)
    {
        this->m_dev_trayShow = trayShow;
        Q_EMIT adapterTrayIconChanged(this->m_dev_trayShow);
    }
}

bool bluetoothadapter::getAdapterTrayShow()
{
    KyDebug();
    return this->m_dev_trayShow;
}

//bluetoothadapter end


//bluetoothdevice
bluetoothdevice::bluetoothdevice(QString         dev_address             ,
                                 QString         dev_name                ,
                                 QString         dev_showName            ,
                                 DEVICE_TYPE     dev_type                ,
                                 bool            dev_paired              ,
                                 bool            dev_trusted             ,
                                 bool            dev_blocked             ,
                                 bool            dev_connected           ,
                                 bool            dev_pairing             ,
                                 bool            dev_connecting          ,
                                 int             dev_battery             ,
                                 int             dev_connectFailedId     ,
                                 QString         dev_connectFailedDisc   ,
                                 qint16          dev_rssi                ,
                                 bool            dev_sendFileMark        ,
                                 QString         adapter_address
                                 )
    :m_dev_address           ( dev_address           )
    ,m_dev_name              ( dev_name              )
    ,m_dev_showName          ( dev_showName          )
    ,m_dev_type              ( dev_type              )
    ,m_dev_paired            ( dev_paired            )
    ,m_dev_trusted           ( dev_trusted           )
    ,m_dev_blocked           ( dev_blocked           )
    ,m_dev_connected         ( dev_connected         )
    ,m_dev_pairing           ( dev_pairing           )
    ,m_dev_connecting        ( dev_connecting        )
    ,m_dev_battery           ( dev_battery           )
    ,m_dev_connectFailedId   ( dev_connectFailedId   )
    ,m_dev_connectFailedDisc ( dev_connectFailedDisc )
    ,m_dev_rssi              ( dev_rssi              )
    ,m_dev_sendFileMark      ( dev_sendFileMark      )
    ,m_adapter_address       ( adapter_address      )
{
    this->setObjectName(dev_address);
}

bluetoothdevice::bluetoothdevice(QMap<QString ,QVariant> value)
{
    bluetoothDeviceDataAnalysis(value);
}

void bluetoothdevice::bluetoothDeviceDataAnalysis(QMap<QString ,QVariant> value)
{
    //解析bluetooth device数据
    QString key = "Paired";
    if(value.contains(key))
        this->m_dev_paired = value[key].toBool();

    key = "Trusted";
    if(value.contains(key))
        this->m_dev_trusted = value[key].toBool();

    key = "Blocked";
    if(value.contains(key))
        this->m_dev_blocked = value[key].toBool();

    key = "Connected";
    if(value.contains(key))
        this->m_dev_connected = value[key].toBool();

    key = "Name";
    if(value.contains(key))
        this->m_dev_name = value[key].toString();

    key = "ShowName";
    if(value.contains(key))
        this->m_dev_showName = value[key].toString();

    key = "Addr";
    if(value.contains(key))
        this->m_dev_address = value[key].toString();

    key = "Type";
    if(value.contains(key))
        this->m_dev_type = bluetoothdevice::DEVICE_TYPE(value[key].toInt());

    key = "Pairing";
    if(value.contains(key))
        this->m_dev_pairing = value[key].toBool();

    key = "Connecting";
    if(value.contains(key))
        this->m_dev_connecting = value[key].toBool();

    key = "Battery";
    if(value.contains(key))
        this->m_dev_battery = value[key].toInt();

    key = "ConnectFailedId";
    if(value.contains(key))
        this->m_dev_connectFailedId = value[key].toInt();

    key = "ConnectFailedDisc";
    if(value.contains(key))
        this->m_dev_connectFailedDisc = value[key].toString();

    key = "Rssi";
    if(value.contains(key))
        this->m_dev_rssi = value[key].toInt();

    key = "FileTransportSupport";
    if(value.contains(key))
        this->m_dev_sendFileMark = value[key].toBool();

    key = "Adapter";
    if(value.contains(key))
        this->m_adapter_address = value[key].toString();

     if (this->m_dev_name.isEmpty())
         this->m_dev_name = this->m_dev_address;
}

void bluetoothdevice::resetDeviceName(QString new_dev_name)
{
    //KyDebug();
    if(new_dev_name != this->m_dev_name)
    {
        this->m_dev_name = new_dev_name;
        Q_EMIT nameChanged(new_dev_name);
    }
}

void bluetoothdevice::setDevShowName(QString new_dev_showName)
{
    //KyDebug() << this->m_dev_showName << new_dev_showName ;
    if(new_dev_showName != this->m_dev_showName)
    {
        this->m_dev_showName = new_dev_showName;
        Q_EMIT showNameChanged(new_dev_showName);
    }
}

QString bluetoothdevice::getDevShowName()
{
    return this->m_dev_showName;
}

QString bluetoothdevice::getDevInterfaceShowName()
{
    if(this->m_dev_showName.isEmpty())
        return this->m_dev_name;
    else
        return this->m_dev_showName;
}

void bluetoothdevice::devMacAddressChanged(QString macAddress)
{
    if(this->m_dev_address != macAddress)
    {
        this->m_dev_address = macAddress;
        //emit macAddressChanged(value);
    }
}

QString bluetoothdevice::getDevName()
{
    //KyDebug();
    return this->m_dev_name;
}

QString bluetoothdevice::getDevAddress()
{
    //KyDebug();
    return this->m_dev_address;
}

void bluetoothdevice::setDevType(DEVICE_TYPE type)
{
    //KyDebug();
    if (this->m_dev_type != type)
    {
        this->m_dev_type = type;
        Q_EMIT typeChanged(type);
    }
}
bluetoothdevice::DEVICE_TYPE bluetoothdevice::getDevType()
{
    //KyDebug();
    return this->m_dev_type;
}

void bluetoothdevice::setDevPairing(bool value)
{
    if (value != this->m_dev_pairing)
    {
        this->m_dev_pairing = value;
        Q_EMIT pairingChanged(value);
    }
}

bool bluetoothdevice::getDevPairing()
{
    return this->m_dev_pairing;
}

void bluetoothdevice::setDevConnecting(bool value)
{
    KyWarning()  << value << this->m_dev_connecting ;
    if (value != this->m_dev_connecting)
    {
        this->m_dev_connecting = value;
        Q_EMIT connectingChanged(value);
    }
}

bool bluetoothdevice::getDevConnecting()
{
    return this->m_dev_connecting;

}

void bluetoothdevice::setDevTrust(bool value)
{
    //KyDebug();
    if (value != this->m_dev_trusted)
    {
        this->m_dev_trusted = value;
        Q_EMIT trustChanged(value);
    }
}
bool bluetoothdevice::getDevTrust()
{
    //KyDebug();
    return this->m_dev_trusted;
}

bool bluetoothdevice::isPaired()
{
    //KyDebug();
    return m_dev_paired;
}
void bluetoothdevice::devPairedChanged(bool value)
{
    qDebug() << Q_FUNC_INFO << value << this->m_dev_paired << __LINE__;
    if(value != this->m_dev_paired )
    {
        this->m_dev_paired = value;
        Q_EMIT pairedChanged(value);
    }
}

bool bluetoothdevice::isConnected()
{
    //KyDebug();
    return m_dev_connected;
}

void bluetoothdevice::devConnectedChanged(bool value)
{
    if(value != this->m_dev_connected)
    {
        this->m_dev_connected = value;
        Q_EMIT connectedChanged(value);
    }
}

void bluetoothdevice::setDevSendFileMark(bool mark)
{
    if (mark != this->m_dev_sendFileMark)
        this->m_dev_sendFileMark = mark;
}
bool bluetoothdevice::getDevSendFileMark()
{
    return this->m_dev_sendFileMark;
}
void bluetoothdevice::setDevRssi(qint16 rssi)
{
    if (m_dev_rssi != rssi)
    {
        this->m_dev_rssi = rssi;
        Q_EMIT this->rssiChanged(rssi);
    }
}

qint16 bluetoothdevice::getDevRssi()
{
    return this->m_dev_rssi;
}


void bluetoothdevice::setDevConnFailedInfo(int errId,QString errInfo)
{
    //if(errId != this->m_dev_connectFailedId)
    {
        this->m_dev_connectFailedId = errId;
    }

    //if(errInfo != this->m_dev_connectFailedDisc)
    {
        this->m_dev_connectFailedDisc = errInfo;

    }
    //收到一次转发一下
    Q_EMIT errorInfoRefresh(errId,errInfo);

}

int bluetoothdevice::getErrorId()
{
    return this->m_dev_connectFailedId;
}

void bluetoothdevice::setRemoving(bool state)
{
    this->m_dev_removing = state;
}
bool bluetoothdevice::getRemoving()
{
    return this->m_dev_removing;
}
//bluetoothdevice end


//

