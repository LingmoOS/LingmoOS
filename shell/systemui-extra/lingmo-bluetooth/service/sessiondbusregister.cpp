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

#include "sessiondbusregister.h"
#include <QMap>
#include <QSet>
#include <QDateTime>
#include <QThread>
#include <QEventLoop>

#include "systemadapter.h"
#include "app.h"
#include "common.h"


#define BLUETOOTH_KEY_VOLUMEDOWN		114
#define BLUETOOTH_KEY_VOLUMEUP		115
#define BLUETOOTH_KEY_NEXTSONG		163
#define BLUETOOTH_KEY_PLAYPAUSE		164
#define BLUETOOTH_KEY_PREVIOUSSONG	165
#define BLUETOOTH_KEY_STOPCD          166
#define BLUETOOTH_KEY_PLAYCD          200
#define BLUETOOTH_KEY_PAUSECD         201


SysDbusMng::SysDbusMng(QObject *parent)
{
    KyInfo();
}

SysDbusMng::~SysDbusMng()
{
    KyInfo();
}

int SysDbusMng::start()
{
    KyInfo();
    m_sys_adapter = new SystemAdapter(this);

    connect(this, &SysDbusMng::adapterAddSignal, m_sys_adapter, &SystemAdapter::adapterAddSignal);
    connect(this, &SysDbusMng::adapterAttrChanged, m_sys_adapter, &SystemAdapter::adapterAttrChanged);
    connect(this, &SysDbusMng::adapterRemoveSignal, m_sys_adapter, &SystemAdapter::adapterRemoveSignal);
    connect(this, &SysDbusMng::deviceAddSignal, m_sys_adapter, &SystemAdapter::deviceAddSignal);
    connect(this, &SysDbusMng::deviceAttrChanged, m_sys_adapter, &SystemAdapter::deviceAttrChanged);
    connect(this, &SysDbusMng::deviceRemoveSignal, m_sys_adapter, &SystemAdapter::deviceRemoveSignal);
    connect(this, &SysDbusMng::ActiveConnection, m_sys_adapter, &SystemAdapter::ActiveConnection);
    connect(this, &SysDbusMng::updateClient, m_sys_adapter, &SystemAdapter::updateClient);
    connect(this, &SysDbusMng::startPair, m_sys_adapter, &SystemAdapter::startPair);
    connect(this, &SysDbusMng::fileStatusChanged, m_sys_adapter, &SystemAdapter::fileStatusChanged);
    connect(this, &SysDbusMng::fileReceiveSignal, m_sys_adapter, &SystemAdapter::fileReceiveSignal);
    connect(this, &SysDbusMng::clearBluetoothDev, m_sys_adapter, &SystemAdapter::clearBluetoothDev);
    connect(this, &SysDbusMng::pingTimeSignal, m_sys_adapter, &SystemAdapter::pingTimeSignal);

    //音频控制信号
    connect(this, &SysDbusMng::VolumeDown, m_sys_adapter, &SystemAdapter::VolumeDown);
    connect(this, &SysDbusMng::VolumeUp, m_sys_adapter, &SystemAdapter::VolumeUp);
    connect(this, &SysDbusMng::Next, m_sys_adapter, &SystemAdapter::Next);
    connect(this, &SysDbusMng::PlayPause, m_sys_adapter, &SystemAdapter::PlayPause);
    connect(this, &SysDbusMng::Previous, m_sys_adapter, &SystemAdapter::Previous);
    connect(this, &SysDbusMng::Stop, m_sys_adapter, &SystemAdapter::Stop);
    connect(this, &SysDbusMng::Play, m_sys_adapter, &SystemAdapter::Play);
    connect(this, &SysDbusMng::Pause, m_sys_adapter, &SystemAdapter::Pause);

    return 0;
}

int SysDbusMng::stop()
{
    return 0;
}

bool SysDbusMng::registerClient(QMap<QString, QVariant> params, QString id)
{
    QString dbusid;
    QString username;
    int type = -1;

    QString key;
    key = AppAttr(enum_app_attr_dbusid);
    if(params.contains(key) && params[key].type() == QVariant::String)
    {
        dbusid = params[key].toString();
    }

    key = AppAttr(enum_app_attr_username);
    if(params.contains(key) && params[key].type() == QVariant::String)
    {
        username = params[key].toString();
    }

    key = AppAttr(enum_app_attr_type);
    if(params.contains(key) && params[key].type() == QVariant::Int)
    {
        type = params[key].toInt();
    }

    KyInfo() << dbusid << username << type;
    if(dbusid.isEmpty() || username.isEmpty() || -1 == type || id != dbusid)
    {
        qWarning() << "registerClient error";
        return false;
    }

    APPMNG::getInstance()->add(dbusid, username, type);

    return true;
}

bool SysDbusMng::unregisterClient(QString dbusid)
{
    return APPMNG::getInstance()->remove(dbusid);
}


QString SysDbusMng::bluetoothKeyValue(unsigned int keyValue, QString str)
{
    //获取当前时间
    long long _currentTime = QDateTime::currentDateTime().toMSecsSinceEpoch();

    //对比按键-按下和释放的间隔时间，判断是否需要处理按键相应
    if ("pressed" == str)
    {
        m_pressTimeMap[keyValue] = _currentTime;
    }
    else if ("released" == str)
    {
        m_releaseTimeMap[keyValue] = _currentTime;
        if (m_pressTimeMap.contains(keyValue))
        {
            if((m_releaseTimeMap[keyValue] - m_pressTimeMap[keyValue]) <= 200)
                sendMultimediaControlButtonSignal(keyValue);
            //时间间隔太长，可能需要特殊处理
        }
        else
        {
            sendMultimediaControlButtonSignal(keyValue);
        }
    }

    return QString("The Bluetooth key value is received.");
}

void SysDbusMng::sendMultimediaControlButtonSignal(unsigned int keyValue)
{
    switch (keyValue)
    {
    case BLUETOOTH_KEY_VOLUMEDOWN:
        emit this->VolumeDown();
        break;
    case BLUETOOTH_KEY_VOLUMEUP:
        emit this->VolumeUp();
        break;
    case BLUETOOTH_KEY_NEXTSONG:
        emit this->Next();
        break;
    case BLUETOOTH_KEY_PLAYPAUSE:
        emit this->PlayPause();
        break;
    case BLUETOOTH_KEY_PREVIOUSSONG:
        emit this->Previous();
        break;
    case BLUETOOTH_KEY_STOPCD:
        emit this->Stop();
        break;
    case BLUETOOTH_KEY_PLAYCD:
        emit this->Play();
        break;
    case BLUETOOTH_KEY_PAUSECD:
        emit this->Pause();
        break;
    default:
        break;
    }
}





