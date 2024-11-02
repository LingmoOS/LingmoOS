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

#include "adapter.h"
#include "config.h"
#include "device.h"
#include "daemon.h"
#include "app.h"
#include "sessiondbusregister.h"



LingmoAdapter::LingmoAdapter(BluezQt::AdapterPtr adapter)
{
    m_adapter = adapter;
    m_set_activeConnection = CONFIG::getInstance()->activeconnection();
    connect(m_adapter.data(),&BluezQt::Adapter::nameChanged,this, &LingmoAdapter::nameChanged);
    connect(m_adapter.data(),&BluezQt::Adapter::poweredChanged,this, &LingmoAdapter::poweredChanged);
    connect(m_adapter.data(),&BluezQt::Adapter::discoverableChanged,this, &LingmoAdapter::discoverableChanged);
    connect(m_adapter.data(),&BluezQt::Adapter::discoveringChanged,this, &LingmoAdapter::discoveringChanged);
    connect(m_adapter.data(),&BluezQt::Adapter::deviceRemoved,this, &LingmoAdapter::deviceRemoved);
    connect(m_adapter.data(),&BluezQt::Adapter::deviceAdded,this, &LingmoAdapter::deviceAdded);
    connect(m_adapter.data(),&BluezQt::Adapter::uuidsChanged, this, &LingmoAdapter::uuidsChanged);
    KyInfo()<< m_adapter->address();
    this->__sendAdd();
}

LingmoAdapter::~LingmoAdapter()
{
    KyInfo() << m_adapter->address();

    this->__clear_timers();

    m_adapter->disconnect(this);
}

int LingmoAdapter::start()
{
    KyInfo() << m_adapter->address();
    m_is_start = true;
    m_need_reconnect = true;
    m_set_power = CONFIG::getInstance()->power_switch();
    m_set_discoverable = CONFIG::getInstance()->discoverable_switch();
    m_set_activeConnection = CONFIG::getInstance()->activeconnection();
    m_active_dev = "";
    m_uuids = m_adapter->uuids();
    this->__init_uuid();
    if(CONFIG::getInstance()->bluetooth_block())
    {
        KyInfo() << "block, not start";
        return -1;
    }


    for(auto item : m_adapter->devices())
    {
        this->add(item);
    }

    if(m_set_power != m_adapter->isPowered())
    {
        this->__setPower();
    }
    else
    {
        this->__setPower();
        emit m_adapter->poweredChanged(m_set_power);
    }

    this->__setDiscoverable();

    m_attr_send[AdapterAttr(enum_Adapter_attr_DefaultAdapter)] = true;
    this->__sendAttr(enum_send_type_immediately);
    return 0;
}

int LingmoAdapter::stop()
{
    KyInfo() << m_adapter->address();
    m_is_start = false;

    m_set_discoverable = false;
    m_set_Discovering = false;

    this->__setDiscoverable();
    this->__setDiscovering();
    this->__clear_timers();

    return 0;
}

int LingmoAdapter::bluetooth_block()
{
    KyInfo() << m_adapter->address();
    m_set_power = false;
    m_set_Discovering = false;
    this->set_need_reconnect(true);

    m_attr_send[AdapterAttr(enum_Adapter_attr_Powered)] = false;
    m_attr_send[AdapterAttr(enum_Adapter_attr_Block)] = true;
    this->__sendAttr(enum_send_type_immediately);

    this->__clear_timers();

    return 0;
}

int LingmoAdapter::reconnect_dev()
{
    KyInfo() << m_adapter->address() << " support_a2dp_source: " << m_support_a2dp_source
             << ", finally_audiodevice:" << CONFIG::getInstance()->finally_audiodevice()
             << ", power: "  << m_adapter->isPowered() << ", is_start: " << m_is_start;
    if(0 == m_reconnect_TimerId && m_support_a2dp_source && !CONFIG::getInstance()->finally_audiodevice().isEmpty()
        && m_adapter->isPowered() && m_is_start)
    {
        m_reconnect_TimerId = this->startTimer(3 * 1000);
    }
    return 0;
}

void LingmoAdapter::nameChanged(const QString &name)
{
    m_attr_send[AdapterAttr(enum_Adapter_attr_Name)] = name;
    this->__sendAttr();
}

void LingmoAdapter::poweredChanged(bool powered)
{
    KyInfo() << m_adapter->address() << powered;
    if(!m_is_start)
    {
        m_attr_send[AdapterAttr(enum_Adapter_attr_Powered)] = powered;
        this->__sendAttr();
        return;
    }

    if(m_set_power != powered && 0 != m_set_TimerID)
    {
        KyWarning() << "set power error, set: "<< m_set_power << ", now: " << powered << ", reset";
        return;
    }

    m_attr_send[AdapterAttr(enum_Adapter_attr_Powered)] = powered;
    this->__sendAttr();

    if(powered)
    {
        this->updateDiscovering();
        if(0 == m_autoclean_TimerID)
        {
            m_autoclean_TimerID = this->startTimer(20 * 1000);
            for(auto iter : m_devices.toStdMap())
            {
                iter.second->reset();
            }
        }

        this->reconnect_dev();

    }
    else
    {
        m_need_reconnect = true;
        //关闭蓝牙后，停止回连
        if(0 != m_reconnect_TimerId)
        {
            this->killTimer(m_reconnect_TimerId);
            m_reconnect_TimerId = 0;
        }

    }

}

void LingmoAdapter::discoverableChanged(bool discoverable)
{
    KyInfo() << m_adapter->address() << discoverable;
    m_attr_send[AdapterAttr(enum_Adapter_attr_Discoverable)] = discoverable;
    this->__sendAttr();
}

void LingmoAdapter::discoveringChanged(bool discovering)
{
    KyInfo() << m_adapter->address() << discovering;
    m_attr_send[AdapterAttr(enum_Adapter_attr_Discovering)] = discovering;
    this->__sendAttr();
}

void LingmoAdapter::deviceAdded(BluezQt::DevicePtr device)
{
    this->add(device);
}

void LingmoAdapter::deviceRemoved(BluezQt::DevicePtr device)
{
    this->remove(device);
}

void LingmoAdapter::uuidsChanged(const QStringList &uuids)
{
    if(!m_is_start)
    {
        KyInfo() << m_adapter->address() << " not default adpater";
        return;
    }
    m_uuids = uuids;
    this->__init_uuid();

}

QStringList LingmoAdapter::getAllDev()
{
    QStringList devlist;
    for(auto item : m_devices.toStdMap())
    {
        devlist.append(item.first);
    }
    return devlist;
}

QStringList LingmoAdapter::getPairedDev()
{
    QStringList devlist;
    for(auto item : m_devices.toStdMap())
    {
        if(item.second->ispaired())
        {
            devlist.append(item.first);
        }
    }
    return devlist;
}

bool LingmoAdapter::setDiscoverable(bool v)
{
    KyInfo() << m_adapter->address() << v;
    m_set_discoverable = v;
    return this->__setDiscoverable();
}

bool LingmoAdapter::setPower(bool v)
{
    KyInfo() << m_adapter->address() << v;
    m_set_power = v;

    if(v)
    {
        DAEMON::getInstance()->unblock_bluetooth();
    }

    if(m_adapter->isPowered() == v)
    {
        KyInfo() << m_adapter->address() << " set same";
        this->__setPower();
        emit m_adapter->poweredChanged(m_set_power);
    }
    else
    {
        KyInfo() << m_adapter->address() << " set not same";
        this->__setPower();
    }

    return 0;
}

bool LingmoAdapter::setActiveConnection(bool v)
{
    KyInfo() << m_adapter->address() << v;
    m_set_activeConnection = v;
    return this->__setActiveConnection();
}

bool LingmoAdapter::setName(QString name)
{
    BluezQt::PendingCall * pending = m_adapter->setName(name);
    //this->wait_for_finish(pending);
    return 0;
}

bool LingmoAdapter::trayShow(bool v)
{
    m_attr_send[AdapterAttr(enum_Adapter_attr_TrayShow)] = v;
    this->__sendAttr();
    return true;
}

QMap<QString, QVariant> LingmoAdapter::getAdapterAttr(QString key)
{
    QMap<QString, QVariant> attrs;
    this->__init_devattr(attrs);
    return attrs;
}

LingmoDevicePtr LingmoAdapter::getDevPtr(QString addr)
{
    if(m_devices.contains(addr))
    {
        return m_devices[addr];
    }
    return nullptr;
}

bool LingmoAdapter::devIsAudioType(QString addr)
{
    if(m_devices.contains(addr))
    {
        return m_devices[addr]->isAudioType();
    }
    return false;
}

int LingmoAdapter::devConnect(QString addr, int type /*= 0*/)
{
    KyInfo() << m_adapter->address() << " addr: "<< addr;
    int ret = ERR_BREDR_CONN_SUC;

    if(addr.isEmpty() || !m_adapter->isPowered())
    {
        ret = ERR_BREDR_UNKNOWN_Other;
        return ret;
    }

    if(m_connectProgress)
    {
        ret =  ERR_BREDR_INTERNAL_Operation_Progress;
        return ret;
    }

    m_connectProgress = true;
    this->updateDiscovering();

    if(addr == m_connected_audio_device)
    {
        KyInfo() << addr << " is connected_audio_device";
        ret = ERR_BREDR_INTERNAL_Already_Connected;
        goto END;
    }

    if(!m_connected_audio_device.isEmpty() &&
        m_devices.contains(m_connected_audio_device) &&
        m_devices.contains(addr) && m_devices[addr]->isAudioType())
    {
        m_devices[m_connected_audio_device]->devDisconnect();
    }

    if(m_devices.contains(addr))
    {
        ret = m_devices[addr]->devConnect(type);
    }
    else
    {
        ret = ERR_BREDR_INTERNAL_Dev_Not_Exist;
    }

END:
    m_connectProgress = false;
    this->updateDiscovering();

    return ret;
}

int LingmoAdapter::devDisconnect(QString addr)
{
    KyInfo() << addr;
    int ret = ERR_BREDR_INTERNAL_Dev_Not_Exist;
    if(m_devices.contains(addr))
    {
        ret = m_devices[addr]->devDisconnect();
    }

    return ret;
}

int LingmoAdapter::devRemove(QStringList devlist)
{
    if(devlist.size() == 0 )
    {
        return 0;
    }

    m_cleanProgress = true;
    this->updateDiscovering();

    for(auto iter : devlist)
    {
        this->__devRemove(iter.toUpper());
        if(iter.toUpper() == CONFIG::getInstance()->finally_audiodevice())
        {
            CONFIG::getInstance()->finally_audiodevice("");
        }
    }

    m_cleanProgress = false;
    this->updateDiscovering();
    return 0;
}

int LingmoAdapter::__devRemove(QString addr)
{
    BluezQt::DevicePtr devPtr = m_adapter->deviceForAddress(addr);
    if(devPtr)
    {
        m_adapter->removeDevice(devPtr);
    }
    return 0;
}

int LingmoAdapter::updateDiscovering()
{
    bool new_set_Discovering = false;

    bool ispower = m_adapter->isPowered();
    int controlnum = APPMNG::getInstance()->getControlNum();
    int allnum = APPMNG::getInstance()->getAllNum();
    KyInfo() << m_adapter->address() << " adapterpower: "<< ispower << " controlnum: " << controlnum
           <<" allappnum: "<< allnum << " connectProgress:" << m_connectProgress << " activeConnection: "
          << m_set_activeConnection << " cleanProgress: " << m_cleanProgress;
    //默认适配器 && 适配器开启 && 无正在连接设备 && 无正在清理设备
    if(m_is_start && ispower && !m_connectProgress && !m_cleanProgress)
    {
        if(controlnum > 0)
        {
            new_set_Discovering = true;
        }
        else if( allnum > 0 && m_set_activeConnection)
        {
            new_set_Discovering = true;
        }
    }

    if(new_set_Discovering != m_set_Discovering)
    {
        m_set_Discovering = new_set_Discovering;
        this->__setDiscovering();
    }

    return 0;
}

int LingmoAdapter::devDisconnectAll()
{
    for(auto iter : m_devices.toStdMap())
    {
        iter.second->devDisconnect();
    }

    return 0;
}

void LingmoAdapter::connected_audio_device(QString v)
{
    KyInfo() << m_adapter->address() << v;
    m_connected_audio_device = v;
}

int LingmoAdapter::activeConnectionReply(QString addr, bool v)
{
    if(addr != m_active_dev || addr.isEmpty() || m_active_dev.isEmpty())
    {
        return -1;
    }

    if(m_devices.contains(addr))
    {
        if(0 != m_activeConnection_TimerID)
        {
            this->killTimer(m_activeConnection_TimerID);
            m_activeConnection_TimerID = 0;
        }

        if(v)
        {
            int ret = this->devConnect(addr);
            if(ERR_BREDR_CONN_SUC == ret )
            {
                CONFIG::getInstance()->finally_audiodevice(addr);
            }
        }
        else
        {
            m_black_activeconn_dev.insert(addr);
        }

        m_active_dev = "";
    }

    return 0;
}

void LingmoAdapter::clearPinCode()
{
    KyInfo() << m_adapter->address();
    m_attr_send[AdapterAttr(enum_Adapter_attr_ClearPinCode)] = true;
    this->__sendAttr();
}

int LingmoAdapter::add(BluezQt::DevicePtr device)
{
    if(m_devices.contains(device->address()))
    {
        m_devices[device->address()]->reset();
    }
    else
    {
        LingmoDevicePtr a(new LingmoDevice(device, m_adapter->address()));
        m_devices[device->address()] = a;
        a->start();
    }
    return 0;
}

int LingmoAdapter::remove(BluezQt::DevicePtr device)
{
    KyInfo() << m_adapter->address() << device->address();
    if(m_devices.contains(device->address()))
    {
        //LingmoDevicePtr对象删除可能延后，不可控，先发送删除信号
        m_devices[device->address()]->__send_remove();
        m_devices.remove(device->address());
    }
    return 0;
}

bool LingmoAdapter::deal_active_connection(QString addr, int timeout)
{
    if(m_black_activeconn_dev.contains(addr) || 0 != m_activeConnection_TimerID)
    {
        KyInfo() << m_adapter->address() << " black list in" << addr;
        return false;
    }

    if(!m_active_dev.isEmpty() || !m_devices.contains(addr))
    {
        return false;
    }

    m_active_dev = addr;
    m_activeConnection_TimerID = this->startTimer(timeout * 1000);
    return true;
}

bool LingmoAdapter::__setDiscoverable()
{
    BluezQt::PendingCall * pending = m_adapter->setDiscoverable(m_set_discoverable);
    //this->wait_for_finish(pending);
    if(0 == m_set_TimerID)
    {
       m_set_TimerID = this->startTimer(5000);
    }

    return true;
}

bool LingmoAdapter::__setPower()
{
    m_powerProgress = true;

    if(!m_set_power)
    {
        m_set_Discovering = false;
        this->__setDiscovering();
    }
    KyInfo() << m_set_power;
    BluezQt::PendingCall * pending = m_adapter->setPowered(m_set_power);
    //this->wait_for_finish(pending);

    m_powerProgress = false;

    if(0 == m_set_TimerID)
    {
       m_set_TimerID = this->startTimer(5000);
    }

    return true;
}

bool LingmoAdapter::__setActiveConnection()
{
    this->updateDiscovering();
    m_attr_send[AdapterAttr(enum_Adapter_attr_ActiveConnection)] = m_set_activeConnection;
    this->__sendAttr();
    return true;
}

bool LingmoAdapter::__setDiscovering()
{
    BluezQt::PendingCall * pending = nullptr;
    if(m_set_Discovering)
    {
        pending = m_adapter->startDiscovery();
    }
    else
    {
        pending = m_adapter->stopDiscovery();
    }
    this->wait_for_finish(pending);

    if(0 == m_set_TimerID)
    {
       m_set_TimerID = this->startTimer(5000);
    }

    return true;
}

void LingmoAdapter::__sendAttr(enum_send_type stype /*= enum_send_type_delay*/)
{
    if(enum_send_type_delay == stype)
    {
        if(0 == m_attrsignal_TimerID)
        {
            m_attrsignal_TimerID = this->startTimer(500);
        }
    }
    else
    {
        if(0 != m_attrsignal_TimerID)
        {
            this->killTimer(m_attrsignal_TimerID);
            m_attrsignal_TimerID = 0;
        }
        KyInfo() << m_adapter->address() <<" attrs: "<< m_attr_send;
        emit SYSDBUSMNG::getInstance()->adapterAttrChanged(m_adapter->address(), m_attr_send);
        m_attr_send.clear();
    }

}

void LingmoAdapter::__sendAdd()
{
    QMap<QString, QVariant> attrs;
    this->__init_devattr(attrs);
    KyDebug() << m_adapter->address() << attrs;
    emit SYSDBUSMNG::getInstance()->adapterAddSignal(attrs);
}

void LingmoAdapter::__reconnectFunc()
{
    KyInfo() << m_need_reconnect;
    if(!m_need_reconnect || !m_adapter->isPowered())
    {
        return;
    }

    m_need_reconnect = false;

    m_reconnectProgress = true;

    KyInfo() << m_adapter->address() << " reconnect: "<< CONFIG::getInstance()->finally_audiodevice();
    this->devConnect(CONFIG::getInstance()->finally_audiodevice(), 1);

    m_reconnectProgress = false;
}

void LingmoAdapter::__clear_timers()
{
    if(0 != m_set_TimerID)
    {
        this->killTimer(m_set_TimerID);
        m_set_TimerID = 0;
    }

    if(0 != m_autoclean_TimerID)
    {
        this->killTimer(m_autoclean_TimerID);
        m_autoclean_TimerID = 0;
    }

    if(0 != m_attrsignal_TimerID)
    {
        this->killTimer(m_attrsignal_TimerID);
        m_attrsignal_TimerID = 0;

        KyDebug() << m_adapter->address() << " clear attrsignal_TimerID " << m_attr_send;
        emit SYSDBUSMNG::getInstance()->adapterAttrChanged(m_adapter->address(), m_attr_send);
        m_attr_send.clear();
    }

    if(0 != m_activeConnection_TimerID)
    {
        this->killTimer(m_activeConnection_TimerID);
        m_activeConnection_TimerID = 0;
        m_active_dev = "";
    }

    if(0 != m_reconnect_TimerId)
    {
        this->killTimer(m_reconnect_TimerId);
        m_reconnect_TimerId = 0;
    }
}

void LingmoAdapter::timerEvent(QTimerEvent *event)
{
    if(event->timerId() == m_set_TimerID)
    {
        KyInfo() << m_adapter->address() << " setAdapterConfTimer ";
        this->killTimer(event->timerId());
        m_set_TimerID = 0;

        if(CONFIG::getInstance()->bluetooth_block())
        {
            KyInfo() << "block, quit";
            return ;
        }

        if(m_set_power != m_adapter->isPowered())
        {
            this->__setPower();
        }
        if(m_set_discoverable != m_adapter->isDiscoverable())
        {
            this->__setDiscoverable();
        }
        if(m_set_Discovering != m_adapter->isDiscovering())
        {
            this->__setDiscovering();
        }
    }
    else if(event->timerId() == m_attrsignal_TimerID)
    {
        KyDebug() << m_adapter->address() << " attrsignalTimer  " << m_attr_send;
        this->killTimer(event->timerId());
        m_attrsignal_TimerID = 0;
        emit SYSDBUSMNG::getInstance()->adapterAttrChanged(m_adapter->address(), m_attr_send);
        m_attr_send.clear();
    }
    else if(event->timerId() == m_autoclean_TimerID)
    {
        //KyDebug() << "auto clean dev";
        QStringList devlist;

        for(auto iter : m_devices.toStdMap())
        {
            if(iter.second->needClean())
            {
                devlist.append(iter.first);
            }
        }

        if(devlist.size() > 0)
        {
            int controlnum = APPMNG::getInstance()->getControlNum();
            if(0 == controlnum)
            {
                this->devRemove(devlist);
            }
            else
            {
                emit SYSDBUSMNG::getInstance()->clearBluetoothDev(devlist);
            }
        }
    }
    else if(event->timerId() == m_activeConnection_TimerID)
    {
        KyInfo() << m_adapter->address() << " activeConnectionTimeout, addr: "  << m_active_dev;
        this->killTimer(event->timerId());
        m_activeConnection_TimerID = 0;
        m_active_dev = "";
    }
    else if(event->timerId() == m_reconnect_TimerId)
    {
        KyInfo() << m_adapter->address() << " reconnectFunc";
        this->killTimer(event->timerId());
        m_reconnect_TimerId = 0;
        this->__reconnectFunc();
    }
    else
    {
        KyInfo() << m_adapter->address() << " other timer, delete";
        this->killTimer(event->timerId());
    }
}

void LingmoAdapter::wait_for_finish(BluezQt::PendingCall *call)
{
    QEventLoop eventloop;
    connect(call, &BluezQt::PendingCall::finished, this, [&](BluezQt::PendingCall *callReturn)
    {
        eventloop.exit();
    });
    eventloop.exec();
}

void LingmoAdapter::__init_uuid()
{
    if(support_a2dp_sink(m_uuids))
    {
        if(!m_support_ad2p_sink)
        {
            m_support_ad2p_sink = true;
            KyInfo() << m_adapter->address() << "support_a2dp_sink";
        }
    }
    else
    {
        if(m_support_ad2p_sink)
        {
            m_support_ad2p_sink = false;
            KyInfo() << m_adapter->address() << "not support_a2dp_sink";
        }
    }

    if(support_a2dp_source(m_uuids))
    {
        if(!m_support_a2dp_source)
        {
            m_support_a2dp_source = true;
            KyInfo() << m_adapter->address() << "support_a2dp_source"
                     << ", devDisconnect finally_audiodevice "
                     << CONFIG::getInstance()->finally_audiodevice();

            this->devDisconnect(CONFIG::getInstance()->finally_audiodevice());

            m_need_reconnect = true;
            this->reconnect_dev();
        }
    }
    else
    {
        if(m_support_a2dp_source)
        {
            m_support_a2dp_source = false;
            KyInfo() << m_adapter->address() << "not support_a2dp_source";
        }
    }

    if(support_hfphsp_ag(m_uuids))
    {
        if(!m_support_hfphsp_ag)
        {
            m_support_hfphsp_ag = true;
            KyInfo() << m_adapter->address() << "support_hfphsp_ag";
        }
    }
    else
    {
        if(m_support_hfphsp_ag)
        {
            m_support_hfphsp_ag = false;
            KyInfo() << m_adapter->address() << "not support_hfphsp_ag";
        }
    }

    if(support_hfphsp_hf(m_uuids))
    {
        if(!m_support_hfphsp_hf)
        {
            m_support_hfphsp_hf = true;
            KyInfo() << m_adapter->address() << "support_hfphsp_hf";
        }
    }
    else
    {
        if(m_support_hfphsp_hf)
        {
            m_support_hfphsp_hf = false;
            KyInfo() << m_adapter->address() << "not support_hfphsp_hf";
        }
    }


    if(support_filetransport(m_uuids))
    {
        if(!m_support_filetransport)
        {
            m_support_filetransport = true;
            KyInfo() << m_adapter->address() << "support_filetransport";
            m_attr_send[AdapterAttr(enum_Adapter_attr_FileTransportSupport)] = m_support_filetransport;
            this->__sendAttr();
        }
    }
    else
    {
        if(m_support_filetransport)
        {
            m_support_filetransport = false;
            KyInfo() << m_adapter->address() << "not support_filetransport";
            m_attr_send[AdapterAttr(enum_Adapter_attr_FileTransportSupport)] = m_support_filetransport;
            this->__sendAttr();
        }
    }
}

void LingmoAdapter::__init_devattr(QMap<QString, QVariant> & attrs)
{
    attrs.clear();
    attrs[AdapterAttr(enum_Adapter_attr_Powered)] = m_adapter->isPowered();
    attrs[AdapterAttr(enum_Adapter_attr_Discoverable)] = m_adapter->isDiscoverable();
    attrs[AdapterAttr(enum_Adapter_attr_Discovering)] = m_adapter->isDiscovering();
    attrs[AdapterAttr(enum_Adapter_attr_Name)] = m_adapter->name();
    attrs[AdapterAttr(enum_Adapter_attr_Addr)] = m_adapter->address();
    attrs[AdapterAttr(enum_Adapter_attr_ActiveConnection)] = m_set_activeConnection;
    attrs[AdapterAttr(enum_Adapter_attr_Block)] = CONFIG::getInstance()->bluetooth_block();
    attrs[AdapterAttr(enum_Adapter_attr_DefaultAdapter)] = m_is_start;
    attrs[AdapterAttr(enum_Adapter_attr_TrayShow)] = CONFIG::getInstance()->trayShow();
    attrs[AdapterAttr(enum_Adapter_attr_FileTransportSupport)] = m_support_filetransport;
}


////////////////////////////////////////////////////////////

AdapterMng::AdapterMng()
{

}

AdapterMng::~AdapterMng()
{
    m_adapters.clear();
}

int AdapterMng::add(BluezQt::AdapterPtr adapter)
{
    if(m_adapters.contains(adapter->address()))
    {
        KyInfo() << adapter->address() << "exist";
        m_adapters.remove(adapter->address());
        this->update_adapter();

        LingmoAdapterPtr a(new LingmoAdapter(adapter));
        m_adapters[adapter->address()] = a;
        this->update_adapter();

        //新增适配器非默认适配器
        if(m_default_adapter && m_default_adapter->addr() != adapter->address())
        {
            a->stop();
        }
    }
    else
    {
        LingmoAdapterPtr a(new LingmoAdapter(adapter));
        m_adapters[adapter->address()] = a;
        this->update_adapter();

        //新增适配器非默认适配器
        if(m_default_adapter && m_default_adapter->addr() != adapter->address())
        {
            a->stop();
        }

        return 0;
    }

    return 0;
}


int AdapterMng::remove(BluezQt::AdapterPtr adapter)
{
    KyInfo() << adapter->address();
    if(m_adapters.contains(adapter->address()))
    {
        m_adapters.remove(adapter->address());

        KyDebug() << adapter->address();
        //LingmoAdapterPtr对象删除可能滞后，先发送移除信号
        emit SYSDBUSMNG::getInstance()->adapterRemoveSignal(adapter->address());
    }

    this->update_adapter();

    return 0;
}

QStringList AdapterMng::getAllAdapterAddress()
{
    QStringList AdapterList;
    for(auto item : m_adapters.toStdMap())
    {
        AdapterList.append(item.first);
    }
    return AdapterList;
}

QStringList AdapterMng::getDefaultAdapterAllDev()
{
    if(m_default_adapter)
    {
        return m_default_adapter->getAllDev();
    }
    return QStringList();
}

QStringList AdapterMng::getDefaultAdapterPairedDev()
{
    if(m_default_adapter)
    {
        return m_default_adapter->getPairedDev();
    }
    return QStringList();
}

int AdapterMng::setDefaultAdapter(QString addr)
{
    if(m_adapters.contains(addr))
    {
        CONFIG::getInstance()->adapter_addr(addr);
        this->update_adapter();
        if(m_default_adapter && m_default_adapter->addr() == addr)
        {
            return 0;
        }
        else
        {
            return -1;
        }
    }
    else
    {
        return -2;
    }
    return 0;
}

QMap<QString, QVariant> AdapterMng::getAdapterAttr(QString addr, QString key)
{
    if(m_adapters.contains(addr))
    {
        return m_adapters[addr]->getAdapterAttr(key);
    }
    return QMap<QString, QVariant>();
}

void AdapterMng::bluetooth_block(bool v)
{
    m_bluetooth_block = v;
    if(m_default_adapter)
    {
        if(v)
        {
            m_default_adapter->bluetooth_block();
        }
        else
        {
            m_default_adapter->start();
        }
    }
}

bool AdapterMng::isDefaultAdapter(QString addr)
{
    if(m_default_adapter && m_default_adapter->addr() == addr)
    {
        return true;
    }
    return false;
}

int AdapterMng::update_adapter()
{
    QString old_addr;
    if(m_default_adapter)
    {
        old_addr = m_default_adapter->addr();
        if(!m_adapters.contains(m_default_adapter->addr()))
        {
            m_default_adapter = nullptr;
        }
    }

    if(m_adapters.size() == 0)
    {
        m_default_adapter = nullptr;
        CONFIG::getInstance()->adapter_addr("null");
    }
    else if(m_adapters.size() == 1)
    {
        m_default_adapter = m_adapters.begin().value();
        CONFIG::getInstance()->adapter_addr(m_default_adapter->addr());
        m_default_adapter->start();
    }
    else
    {
        QString conf_addr =  CONFIG::getInstance()->adapter_addr();
        if(m_default_adapter)
        {
            if(m_default_adapter->addr() != conf_addr)
            {
                if(m_adapters.contains(conf_addr))
                {
                    m_default_adapter->stop();
                    m_default_adapter = m_adapters[conf_addr];
                    m_default_adapter->start();
                }
                else
                {
                    CONFIG::getInstance()->adapter_addr(m_default_adapter->addr());
                }
            }
        }
        else
        {
            if(m_adapters.contains(conf_addr))
            {
                m_default_adapter = m_adapters[conf_addr];
            }
            else
            {
                m_default_adapter = m_adapters.begin().value();
                CONFIG::getInstance()->adapter_addr(m_default_adapter->addr());
            }
            m_default_adapter->start();
        }
    }

    if(m_default_adapter && m_default_adapter->addr() != old_addr)
    {
        KyInfo() << "default adapter changed, old: "<< old_addr
               <<", new: " << m_default_adapter->addr();
    }

    return 0;
}


