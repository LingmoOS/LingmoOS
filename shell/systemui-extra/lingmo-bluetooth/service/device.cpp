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

#include "device.h"
#include <QThread>
#include <QEventLoop>
#include <QTimerEvent>

#include "sessiondbusregister.h"
#include "adapter.h"
#include "config.h"


//class BluezQt::MediaTransport{};

LingmoDevice::LingmoDevice(BluezQt::DevicePtr device, QString adapter)
{
    m_device = device;
    m_adapter = adapter;
    connect(m_device.data(), &BluezQt::Device::nameChanged, this, &LingmoDevice::nameChanged);
    connect(m_device.data(), &BluezQt::Device::pairedChanged, this, &LingmoDevice::pairedChanged);
    connect(m_device.data(), &BluezQt::Device::trustedChanged, this, &LingmoDevice::trustedChanged);
    connect(m_device.data(), &BluezQt::Device::blockedChanged, this, &LingmoDevice::blockedChanged);
    connect(m_device.data(), &BluezQt::Device::rssiChanged, this, &LingmoDevice::rssiChanged);
    connect(m_device.data(), &BluezQt::Device::connectedChanged, this, &LingmoDevice::connectedChanged);
#ifdef BATTERY
    connect(m_device.data(), &BluezQt::Device::batteryChanged, this, &LingmoDevice::batteryChanged);
#endif
    connect(m_device.data(), &BluezQt::Device::typeChanged, this, &LingmoDevice::typeChanged);
    connect(m_device.data(), &BluezQt::Device::uuidsChanged, this, &LingmoDevice::uuidsChanged);
    connect(m_device.data(), &BluezQt::Device::mediaTransportChanged, this, &LingmoDevice::mediaTransportChanged);
    m_a2dp_connect =m_device->mediaTransport() ? true : false;
    KyDebug() << m_device->address() << " a2dp_connect : " << m_a2dp_connect;
    m_paired = m_device->isPaired();
    if(m_paired)
    {
        m_connect = m_device->isConnected();
    }
    m_uuids = m_device->uuids();
    m_showName = CONFIG::getInstance()->get_rename(m_device->address());
    this->__send_add();
    this->__init_uuid();
}

LingmoDevice::~LingmoDevice()
{
    KyDebug() << m_device->address();
    m_device->disconnect(this);

    if(0 != m_attrsignal_TimerID)
    {
        this->killTimer(m_attrsignal_TimerID);
        m_attrsignal_TimerID = 0;
    }

    this->__kill_reconnect_a2dp_timer();
}

int LingmoDevice::start()
{
    this->deal_active_connection();
    return 0;
}

int LingmoDevice::devConnect(int type /*= 0*/)
{
    KyInfo()<< m_device->address();
    m_need_clean = 0;
    m_lastConnectError = "";
    m_connect_type = type;
    if(m_connectProgress)
    {
        return ERR_BREDR_INTERNAL_Operation_Progress;
    }

    m_connectProgress = true;
    m_attr_send[DeviceAttr(enum_device_attr_Connecting)] = m_connectProgress;
    this->__send_attr(enum_send_type_immediately);

    this->__kill_reconnect_a2dp_timer();

    int ret = ERR_BREDR_CONN_SUC;

    if(m_device->isPaired())
    {
        if(Environment::HUAWEI == envPC)
        {
            if(m_device->type() != BluezQt::Device::Computer && m_device->type() != BluezQt::Device::Phone)
            {
                ret = this->connectToDevice();
            }
        }
        else
        {
            ret = this->connectToDevice();
        }
    }
    else
    {
        ret = this->pair();
        if(ERR_BREDR_CONN_SUC == ret)
        {
            if(Environment::HUAWEI == envPC)
            {
                if(m_device->type() != BluezQt::Device::Computer && m_device->type() != BluezQt::Device::Phone)
                {
                    ret = this->connectToDevice();
                }
            }
            else
            {
                ret = this->connectToDevice();
            }
        }
    }

    m_connectProgress = false;
    m_attr_send[DeviceAttr(enum_device_attr_Connecting)] = m_connectProgress;
    this->__send_attr(enum_send_type_immediately);

    return ret;
}

int LingmoDevice::devDisconnect()
{
    KyInfo()<< m_device->address();
    m_need_clean = 0;

    this->__kill_reconnect_a2dp_timer();

    if((m_device->isPaired() && m_device->isConnected()) || m_connect)
    {
        BluezQt::PendingCall *call = m_device->disconnectFromDevice();
        //this->wait_for_finish(call);
    }
    return ERR_BREDR_CONN_SUC;
}

int LingmoDevice::pairFuncReply(bool v)
{
    KyInfo()<< m_device->address() << v;
    if(v)
    {
        m_request.accept();
    }
    else
    {
        m_request.reject();
    }
    return 0;
}

void LingmoDevice::requestPinCode(const BluezQt::Request<QString> &request)
{
    KyInfo()<< m_device->address();
    request.accept(QString());
}

void LingmoDevice::requestPasskey(const BluezQt::Request<quint32> &request)
{
    KyInfo()<< m_device->address();
    request.accept(0);
}

void LingmoDevice::requestConfirmation(const QString &passkey, const BluezQt::Request<> &request)
{
    KyInfo()<< m_device->address();
    if(this->isAudioType())
    {
        request.accept();
    }
    else
    {
        m_request = request;
        QMap<QString, QVariant> attrs;
        attrs[PairAttr(enum_start_pair_dev)] = m_device->address();
        attrs[PairAttr(enum_start_pair_name)] = m_device->name();
        attrs[PairAttr(enum_start_pair_pincode)] = passkey;
        attrs[PairAttr(enum_start_pair_type)] = enum_pair_pincode_type_request;
        emit SYSDBUSMNG::getInstance()->startPair(attrs);

        //request.accept();
    }
}

void LingmoDevice::requestAuthorization(const BluezQt::Request<> &request)
{
    KyInfo()<< m_device->address();
    request.accept();
}

void LingmoDevice::authorizeService(const QString &uuid, const BluezQt::Request<> &request)
{
    KyInfo()<< m_device->address() << uuid;
    if(this->isAudioType())
    {
        //主动连接， 信任状态
        if(m_device->isTrusted())
        {
            request.accept();
        }
        //本机移除后再连接
        else
        {
            request.reject();
        }
    }
    else
    {
        request.accept();
        m_connect = true;
        m_attr_send[DeviceAttr(enum_device_attr_Connected)] = m_connect;
        this->__send_attr();
    }
}

void LingmoDevice::displayPinCode(const QString &pinCode)
{
    KyInfo()<< m_device->address();
    //if(m_connectProgress)
    {
        QMap<QString, QVariant> attrs;
        attrs[PairAttr(enum_start_pair_dev)] = m_device->address();
        attrs[PairAttr(enum_start_pair_name)] = m_device->name();
        attrs[PairAttr(enum_start_pair_pincode)] = pinCode;
        attrs[PairAttr(enum_start_pair_type)] = enum_pair_pincode_type_display;
        emit SYSDBUSMNG::getInstance()->startPair(attrs);
    }
}

bool LingmoDevice::needClean()
{
    if(m_connect || m_paired || m_connectProgress || m_pairProgress || m_device->isPaired() || m_device->isConnected())
    {
        return false;
    }

    if(m_need_clean > 3)
    {
        KyDebug() << m_device->address() << "need delete";
        return true;
    }

    m_need_clean++;

    return false;
}

bool LingmoDevice::isAudioType()
{
    if(m_device->type() == BluezQt::Device::AudioVideo ||
       m_device->type() == BluezQt::Device::Headphones ||
       m_device->type() == BluezQt::Device::Headset)
    {
        return true;
    }

    return false;
}

int LingmoDevice::reset()
{
    m_need_clean = 0;
    return 0;
}

void LingmoDevice::setSendfileStatus(bool status /*= true*/)
{
    KyInfo() << "connect: " << m_connect << "  devconnect: " << m_device->isConnected() << " stasus: " << status;
    if (status) {
        if (!m_connect) {
            m_connect = true;
            m_attr_send[DeviceAttr(enum_device_attr_Connected)] = m_connect;
            this->__send_attr();
        }
    }
    else {
        if (m_connect != m_device->isConnected()) {
            m_connect = m_device->isConnected();
            m_attr_send[DeviceAttr(enum_device_attr_Connected)] = m_connect;
            this->__send_attr();
        }
    }
}

void LingmoDevice::nameChanged(const QString &name)
{
    m_need_clean = 0;
    m_attr_send[DeviceAttr(enum_device_attr_Name)] = name;
    this->__send_attr();
}

void LingmoDevice::pairedChanged(bool paired)
{
    m_need_clean = 0;
    KyInfo() << m_device->address() <<", paired: "<< paired;
    if(this->isAudioType())
    {
        if(m_pairProgress || m_paired)
        {
            m_paired = paired;
            m_attr_send[DeviceAttr(enum_device_attr_Paired)] = paired;
            this->__send_attr();
        }
        else
        {
            //移除设备
            LingmoAdapterPtr adapterPtr = ADAPTERMNG::getInstance()->getDefaultAdapter();
            if(adapterPtr)
            {
                KyInfo() << m_device->address() << "active connect, delete dev";
                adapterPtr->__devRemove(m_device->address());
            }
        }
    }
    else
    {
        m_paired = paired;
        m_attr_send[DeviceAttr(enum_device_attr_Paired)] = paired;
        this->__send_attr();
    }

}

void LingmoDevice::trustedChanged(bool trusted)
{
    m_need_clean = 0;
    m_attr_send[DeviceAttr(enum_device_attr_Trusted)] = trusted;
    this->__send_attr();
}

void LingmoDevice::blockedChanged(bool blocked)
{
    m_need_clean = 0;
}

void LingmoDevice::rssiChanged(qint16 rssi)
{
    m_need_clean = 0;
    if(-rssi > 0 && -rssi < 100 )
    {
        //this->deal_active_connection();
        m_attr_send[DeviceAttr(enum_device_attr_Rssi)] = rssi;
        this->__send_attr();
    }
}

void LingmoDevice::connectedChanged(bool connected)
{
    m_need_clean = 0;
    KyInfo() << m_device->address() << connected;

    if(Environment::HUAWEI == envPC)
    {
        if(BluezQt::Device::Computer == m_device->type() ||
           BluezQt::Device::Phone == m_device->type() )
        {
            if(connected && m_device->isPaired())
            {
                KyInfo() << m_device->address() << " disconnect, HUAWEI";
                m_device->disconnectFromDevice();
                return;
            }
        }
    }

    // 底层状态断开  && 当前保存状态 不等于 底层状态
    if(!connected && m_connect != connected)
    {
        m_connect = connected;
        m_attr_send[DeviceAttr(enum_device_attr_Connected)] = m_connect;
        this->__send_attr();
        this->updateAudioDevice();

        this->__kill_reconnect_a2dp_timer();
    }
    //未主动连接   && 底层状态变为已连接    && 设备信任(以前主动连接的设备)
    else if(!m_connectProgress && connected && m_device->isTrusted())
    {
        m_connect = connected;
        m_attr_send[DeviceAttr(enum_device_attr_Connected)] = m_connect;
        this->__send_attr();
        this->updateAudioDevice();
    }
}
#ifdef BATTERY
void LingmoDevice::batteryChanged(BluezQt::BatteryPtr battery)
{
    m_need_clean = 0;
    if(battery)
    {
        connect(battery.data(), &BluezQt::Battery::percentageChanged,
                this, &LingmoDevice::percentageChanged, Qt::UniqueConnection);
        m_attr_send[DeviceAttr(enum_device_attr_Battery)] = battery->percentage();
        this->__send_attr();
    }
}

void LingmoDevice::percentageChanged(int percentage)
{
    m_need_clean = 0;
    m_attr_send[DeviceAttr(enum_device_attr_Battery)] = percentage;
    this->__send_attr();
}

#endif
void LingmoDevice::typeChanged(BluezQt::Device::Type Type)
{
    m_need_clean = 0;
    if(Environment::HUAWEI == envPC)
    {
        if(Type == BluezQt::Device::Type::AudioVideo)
        {
            m_attr_send[DeviceAttr(enum_device_attr_Type)] = BluezQt::Device::Type::Headphones;
        }
        else
        {
            m_attr_send[DeviceAttr(enum_device_attr_Type)] = Type;
        }
    }
    else
    {
        m_attr_send[DeviceAttr(enum_device_attr_Type)] = Type;
    }

    m_attr_send[DeviceAttr(enum_device_attr_TypeName)] = BluezQt::Device::typeToString(Type);
    this->__send_attr();
}

void LingmoDevice::uuidsChanged(const QStringList &uuids)
{
    m_uuids = uuids;
    this->__init_uuid();;
}

void LingmoDevice::mediaTransportChanged(BluezQt::MediaTransportPtr mediaTransport)
{
    KyInfo() << m_device->address();
    if(mediaTransport)
    {
        KyInfo() << m_device->address() << " a2dp connect suc";
        this->__kill_reconnect_a2dp_timer();
        m_a2dp_connect = true;
    }
    else
    {
        KyInfo() << m_device->address() << " a2dp disconnect";
        if(m_a2dp_connect)
        {
            this->__kill_reconnect_a2dp_timer();
            m_a2dp_reconnect_TimerID = this->startTimer(5000);
            KyInfo() << m_device->address() << "start a2dp reconnect timer";
        }
        m_a2dp_connect = false;
    }
}

void LingmoDevice::pairfinished(BluezQt::PendingCall *call)
{
    if(call->error() != 0)
    {
        KyWarning()<< m_device->address() <<" paired error, code: "
                  <<call->error() << ", msg: "<< call->errorText();
        BluezQt::PendingCall * p = m_device.data()->cancelPairing();
        this->wait_for_finish(p);
        m_lastConnectError = call->errorText();
        m_attr_send[DeviceAttr(enum_device_attr_ConnectFailedDisc)] = m_lastConnectError;
        m_attr_send[DeviceAttr(enum_device_attr_ConnectFailedId)] = get_enum_errmsg(m_lastConnectError);
        this->__send_attr();
    }
    //配对成功
    else
    {
        m_paired = true;
        KyInfo() << m_device->address() << " pair suc";
        BluezQt::PendingCall * p = m_device->setTrusted(true);
        this->wait_for_finish(p);
    }
}

void LingmoDevice::connectfinished(BluezQt::PendingCall *call)
{
    if(call->error() != 0)
    {
        m_connect = false;
        KyWarning()<< m_device->address() <<" connect error, code: "
                  <<call->error() << ", msg: "<< call->errorText();

        //回连失败不发送错误信号
        if(0 == m_connect_type)
        {
            m_lastConnectError = call->errorText();
            m_attr_send[DeviceAttr(enum_device_attr_ConnectFailedDisc)] = m_lastConnectError;
            m_attr_send[DeviceAttr(enum_device_attr_ConnectFailedId)] = get_enum_errmsg(m_lastConnectError);
            this->__send_attr();
        }
    }
    else
    {
        KyInfo() << m_device->address() << " connect suc";
        m_connect = true;
        m_attr_send[DeviceAttr(enum_device_attr_Connected)] = m_connect;
        this->__send_attr();

        this->updateAudioDevice();
    }
}

int LingmoDevice::pair()
{
    m_pairProgress = true;
    KyInfo()<< m_device->address();
    BluezQt::PendingCall *call = m_device->pair();
    connect(call, &BluezQt::PendingCall::finished, this, &LingmoDevice::pairfinished);
    this->wait_for_finish(call);
    m_pairProgress = false;

    if(m_lastConnectError.isEmpty())
    {
        return 0;
    }
    return get_enum_errmsg(m_lastConnectError);

}

int LingmoDevice::connectToDevice()
{
    KyInfo() << m_device->address();
    BluezQt::PendingCall *call = m_device->connectToDevice();
    connect(call, &BluezQt::PendingCall::finished, this, &LingmoDevice::connectfinished);
    this->wait_for_finish(call);
    if(m_lastConnectError.isEmpty())
    {
        return 0;
    }
    return get_enum_errmsg(m_lastConnectError);
}

int LingmoDevice::updateAudioDevice()
{
    //音频设备
    if(this->isAudioType())
    {
        QString old_connected_audio_device;
        //移除设备
        LingmoAdapterPtr adapterPtr = ADAPTERMNG::getInstance()->getDefaultAdapter();
        if(adapterPtr)
        {
            old_connected_audio_device = adapterPtr->connected_audio_device();
            if(m_connect)
            {
                if(old_connected_audio_device != m_device->address())
                {
                    adapterPtr->connected_audio_device(m_device->address());
                }

                //判断a2dp是否连接
                m_a2dp_connect = m_device->mediaTransport() ? true : false;
                if(!m_a2dp_connect)
                {
                    if(0 != m_a2dp_reconnect_TimerID)
                    {
                        this->__kill_reconnect_a2dp_timer();
                    }
                    m_a2dp_reconnect_TimerID = this->startTimer(5000);
                    KyInfo() << m_device->address() << "start a2dp reconnect timer";
                }

            }
            else
            {
                if(old_connected_audio_device == m_device->address())
                {
                    adapterPtr->connected_audio_device("");
                }
            }
        }
    }
    return 0;
}

void LingmoDevice::deal_active_connection()
{
    if(CONFIG::getInstance()->activeconnection())
    {
        if(!this->isAudioType() || m_device->isPaired())
        {
            return;
        }

        int timeout = 15;
        int v= m_device->rssi();
        if(abs(v) <= 60)
        {
            //移除设备
            LingmoAdapterPtr adapterPtr = ADAPTERMNG::getInstance()->getDefaultAdapter();
            if(adapterPtr && adapterPtr->deal_active_connection(m_device->address(), timeout))
            {
                emit SYSDBUSMNG::getInstance()->ActiveConnection(m_device->address(), m_device->name(),
                     BluezQt::Device::typeToString(m_device->type()), v, timeout);

                KyInfo() <<  "start active conection" << m_device->address()
                        <<" rssi: "<< m_device->rssi() << " timeout:" << timeout
                        <<" type: " << BluezQt::Device::typeToString(m_device->type())
                        <<" name: " << m_device->name();
            }
        }
    }
}

void LingmoDevice::timerEvent(QTimerEvent *event)
{
    if(event->timerId() == m_attrsignal_TimerID)
    {
        KyDebug() << m_device->address() <<"  send attrs" << m_attr_send;
        this->killTimer(m_attrsignal_TimerID);
        m_attrsignal_TimerID = 0;
        emit SYSDBUSMNG::getInstance()->deviceAttrChanged(m_device->address(), m_attr_send);
        m_attr_send.clear();
    }
    else if(event->timerId() == m_a2dp_reconnect_TimerID)
    {
        KyInfo() << m_device->address() <<"  a2dp_reconnect_TimerID";

        if(m_device->isConnected())
        {
            m_a2dp_reconnect_count += 1;
            if(m_a2dp_reconnect_count > 2)
            {
                KyInfo() << m_device->address() <<" a2dp sink connect failed, disconnect";
                this->devDisconnect();
                return;
            }
            KyInfo() << m_device->address() <<"  reconnect a2dp sink";
            m_device->connectProfile(A2DP_SINK_UUID);
        }
        else
        {
            // 蓝牙连接断开，移删除定时器
            this->__kill_reconnect_a2dp_timer();
            KyInfo() << m_device->address() <<"  already disconnected";
        }

    }
    else
    {
        KyDebug() << m_device->address() << "other timer, delete";
        this->killTimer(event->timerId());
    }
}

void LingmoDevice::wait_for_finish(BluezQt::PendingCall *call)
{
    QEventLoop eventloop;
    connect(call, &BluezQt::PendingCall::finished, this, [&](BluezQt::PendingCall *callReturn)
    {
        eventloop.exit();
    });
    eventloop.exec();

}

void LingmoDevice::__send_attr(enum_send_type stype /*= enum_send_type_delay*/)
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
        KyDebug() << m_device->address() << " attrs: "<< m_attr_send;
        emit SYSDBUSMNG::getInstance()->deviceAttrChanged(m_device->address(), m_attr_send);
        m_attr_send.clear();
    }
}

void LingmoDevice::__send_remove()
{
    QMap<QString, QVariant> attrs;
    attrs[DeviceAttr(enum_device_attr_Adapter)] = m_adapter;
    emit SYSDBUSMNG::getInstance()->deviceRemoveSignal(m_device->address(), attrs);
}

void LingmoDevice::__init_uuid()
{
    if(support_a2dp_sink(m_uuids))
    {
        if(!m_support_ad2p_sink)
        {
            m_support_ad2p_sink = true;
            //KyDebug() << m_device->address() <<" support_a2dp_sink";
        }
    }
    else
    {
        if(m_support_ad2p_sink)
        {
            m_support_ad2p_sink = false;
            //KyDebug() << m_device->address() << "not support_a2dp_sink";
        }
    }

    if(support_a2dp_source(m_uuids))
    {
        if(!m_support_a2dp_source)
        {
            m_support_a2dp_source = true;
            //KyDebug()<< m_device->address()  << "support_a2dp_source";
        }
    }
    else
    {
        if(m_support_a2dp_source)
        {
            m_support_a2dp_source = false;
            //KyDebug()<< m_device->address()  << "not support_a2dp_source";
        }
    }

    if(support_hfphsp_ag(m_uuids))
    {
        if(!m_support_hfphsp_ag)
        {
            m_support_hfphsp_ag = true;
            //KyDebug()<< m_device->address() << "support_hfphsp_ag";
        }
    }
    else
    {
        if(m_support_hfphsp_ag)
        {
            m_support_hfphsp_ag = false;
            //KyDebug()<< m_device->address() << "not support_hfphsp_ag";
        }
    }

    if(support_hfphsp_hf(m_uuids))
    {
        if(!m_support_hfphsp_hf)
        {
            m_support_hfphsp_hf = true;
            //KyDebug() << m_device->address() << "support_hfphsp_hf";
        }
    }
    else
    {
        if(m_support_hfphsp_hf)
        {
            m_support_hfphsp_hf = false;
            //KyDebug()<< m_device->address()   << "not support_hfphsp_hf";
        }
    }


    if(support_filetransport(m_uuids))
    {
        if(!m_support_filetransport)
        {
            m_support_filetransport = true;
            m_attr_send[DeviceAttr(enum_device_attr_FileTransportSupport)] = m_support_filetransport;
            this->__send_attr();
            //KyDebug()<< m_device->address()   << "support_filetransport";
        }
    }
    else
    {
        if(m_support_filetransport)
        {
            m_support_filetransport = false;
            m_attr_send[DeviceAttr(enum_device_attr_FileTransportSupport)] = m_support_filetransport;
            this->__send_attr();
            //KyDebug()<< m_device->address() << "not support_filetransport";
        }
    }
}

void LingmoDevice::__init_devattr(QMap<QString, QVariant> & attrs)
{
    attrs.clear();
    attrs[DeviceAttr(enum_device_attr_Adapter)] = m_adapter;
    attrs[DeviceAttr(enum_device_attr_Paired)] = m_device->isPaired();
    attrs[DeviceAttr(enum_device_attr_Trusted)] = m_device->isTrusted();
    attrs[DeviceAttr(enum_device_attr_Connected)] = m_connect;
    attrs[DeviceAttr(enum_device_attr_Name)] = m_device->name();
    if(Environment::HUAWEI == envPC)
    {
        if(m_device->type() == BluezQt::Device::Type::AudioVideo)
        {
            attrs[DeviceAttr(enum_device_attr_Type)] = BluezQt::Device::Type::Headphones;
        }
        else
        {
            attrs[DeviceAttr(enum_device_attr_Type)] = m_device->type();
        }
    }
    else
    {
        attrs[DeviceAttr(enum_device_attr_Type)] = m_device->type();
    }

    attrs[DeviceAttr(enum_device_attr_TypeName)] = BluezQt::Device::typeToString(m_device->type());
    attrs[DeviceAttr(enum_device_attr_Connecting)] = m_connectProgress;
    if(m_device->type() == BluezQt::Device::Type::Phone || m_device->type() == BluezQt::Device::Type::Computer)
    {
        attrs[DeviceAttr(enum_device_attr_FileTransportSupport)] = m_support_filetransport;
    }
#ifdef BATTERY
    if(m_device->battery())
    {
        attrs[DeviceAttr(enum_device_attr_Battery)] = m_device->battery()->percentage();
    }
#endif
    attrs[DeviceAttr(enum_device_attr_Addr)] = m_device->address();
    attrs[DeviceAttr(enum_device_attr_Rssi)] = m_device->rssi();
    attrs[DeviceAttr(enum_device_attr_ShowName)] = m_showName;
}

void LingmoDevice::__kill_reconnect_a2dp_timer()
{
    if(0 != m_a2dp_reconnect_TimerID)
    {
        KyInfo() << m_device->address() << "stop a2dp reconnect timer";
        this->killTimer(m_a2dp_reconnect_TimerID);
        m_a2dp_reconnect_TimerID = 0;
        m_a2dp_reconnect_count = 0;
    }
}

QMap<QString, QVariant> LingmoDevice::getDevAttr()
{
    QMap<QString, QVariant> attrs;
    this->__init_devattr(attrs);
    return attrs;
}

int LingmoDevice::setDevAttr(QMap<QString, QVariant> attrs)
{
    QString key = DeviceAttr(enum_device_attr_Name);
    if(attrs.contains(key) && this->ispaired())
    {
        m_showName = attrs[key].toString();
        if(m_showName.length() == 0)
        {
            m_showName = m_device->name();
        }
        CONFIG::getInstance()->set_rename(m_device->address(), m_showName);
        m_attr_send[DeviceAttr(enum_device_attr_ShowName)] = m_showName;
        this->m_device->setName(m_showName);
        this->__send_attr();
    }
    return 0;
}

void LingmoDevice::__send_add()
{
    QMap<QString, QVariant> attrs;
    this->__init_devattr(attrs);
    KyDebug() << m_device->address()  << attrs;
    emit SYSDBUSMNG::getInstance()->deviceAddSignal(attrs);
}
