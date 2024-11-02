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

#include "daemon.h"
#include <QDateTime>
#include <QFile>
#include "sessiondbusregister.h"
#include "adapter.h"

extern "C" {
    #include <string>
    #include <stdlib.h>
}


Daemon::Daemon(QObject *parent)
{

}

Daemon::~Daemon()
{

}

int Daemon::start()
{
    this->init_bluez();
    this->init_sys_monistor();
    return 0;
}

int Daemon::stop()
{
    return 0;
}

int Daemon::unblock_bluetooth()
{
    if(m_bluez->isBluetoothBlocked())
    {
        m_bluez->setBluetoothBlocked(false);
    }
    return 0;
}


int Daemon::init_bluez()
{
    m_bluez = new BluezQt::Manager(this);
    BluezQt::InitManagerJob *job = m_bluez->init();
    job->exec();

    connect(m_bluez,&BluezQt::Manager::operationalChanged,this, &Daemon::operationalChanged);
    connect(m_bluez,&BluezQt::Manager::bluetoothOperationalChanged,this, &Daemon::bluetoothOperationalChanged);
    connect(m_bluez,&BluezQt::Manager::bluetoothBlockedChanged,this, &Daemon::bluetoothBlockedChanged);
    connect(m_bluez,&BluezQt::Manager::adapterAdded,this, &Daemon::adapterAdded);
    connect(m_bluez,&BluezQt::Manager::adapterRemoved,this, &Daemon::adapterRemoved);
    connect(m_bluez,&BluezQt::Manager::usableAdapterChanged,this, &Daemon::usableAdapterChanged);
    connect(m_bluez,&BluezQt::Manager::allAdaptersRemoved,this, &Daemon::allAdaptersRemoved);

    KyInfo()<< Q_FUNC_INFO << m_bluez->isOperational() << m_bluez->isBluetoothOperational();

    emit m_bluez->operationalChanged(m_bluez->isOperational());
    emit m_bluez->bluetoothBlockedChanged(m_bluez->isBluetoothBlocked());

    if(m_bluez->adapters().size() > 0)
    {
        this->__init_bluez_adapter();
    }

    return 0;
}


int Daemon::init_sys_monistor()
{
    KyInfo();
    //部分机器在系统s3/s4会对蓝牙适配器进行移除添加操作
    if (QDBusConnection::systemBus().connect("org.freedesktop.login1", "/org/freedesktop/login1",
            "org.freedesktop.login1.Manager", "PrepareForSleep", this,
            SLOT(monitorSleepSlot(bool))))
    {
        KyDebug() << "PrepareForSleep signal connected successfully to slot";
    }
    else
    {
        KyDebug() << "PrepareForSleep signal connection was not successful";
    }

    //部分机器在系统s3/s4会对蓝牙适配器进行移除添加操作
    if (QDBusConnection::systemBus().connect("org.lingmo.test", "/com/lingmo/test",
            "com.lingmo.test", "ActiveUserChange", this,
            SLOT(activeUserChange(QString))))
    {
        KyDebug() << "activeUserChange signal connected successfully to slot";
    }
    else
    {
        KyDebug() << "activeUserChange signal connection was not successful";
    }

    return 0;
}

int Daemon::__init_bluez_adapter()
{
    KyInfo();
    if(m_bluez->adapters().size() > 0)
    {
        for (auto item : m_bluez->adapters())
        {
            ADAPTERMNG::getInstance()->add(item);
        }
    }

    return 0;
}

void Daemon::operationalChanged(bool operational)
{
    KyInfo() << operational;
    if(operational)
    {
        if(!BLUETOOTHAGENT::getInstance())
        {
            BLUETOOTHAGENT::instance(this);
            BluezQt::PendingCall * call = m_bluez->registerAgent(BLUETOOTHAGENT::getInstance());
            this->wait_for_finish(call);
            call = m_bluez->requestDefaultAgent(BLUETOOTHAGENT::getInstance());
            this->wait_for_finish(call);
        }
    }
    else
    {
        if(BLUETOOTHAGENT::getInstance())
        {
            BluezQt::PendingCall * call = m_bluez->unregisterAgent(BLUETOOTHAGENT::getInstance());
            this->wait_for_finish(call);
            BLUETOOTHAGENT::destroyInstance();
        }
    }
}

void Daemon::bluetoothOperationalChanged(bool operational)
{
    KyInfo() << operational;
}

void Daemon::bluetoothBlockedChanged(bool blocked)
{
    KyInfo() << blocked;
    CONFIG::getInstance()->bluetooth_block(blocked);
    ADAPTERMNG::getInstance()->bluetooth_block(blocked);
}

void Daemon::adapterAdded(BluezQt::AdapterPtr adapter)
{
    KyInfo() << adapter->address();
    ADAPTERMNG::getInstance()->add(adapter);
}

void Daemon::adapterRemoved(BluezQt::AdapterPtr adapter)
{
    KyInfo() << adapter->address();
    ADAPTERMNG::getInstance()->remove(adapter);
}


void Daemon::usableAdapterChanged(BluezQt::AdapterPtr adapter)
{
    if(adapter.isNull())
    {
        KyInfo();
    }
    else
    {
        KyInfo() << adapter->address();
    }

}

void Daemon::allAdaptersRemoved()
{
    KyInfo();
}


void Daemon::monitorSleepSlot(bool sleep)
{
    KyInfo () << "app is sleep: " << sleep;

    CONFIG::getInstance()->systemSleepFlag(sleep);

    if(sleep)
    {
        LingmoAdapterPtr ptr = ADAPTERMNG::getInstance()->getDefaultAdapter();
        if(ptr)
        {
            ptr->devDisconnectAll();
        }
    }
    else
    {
        //启动默认适配器
        LingmoAdapterPtr ptr = ADAPTERMNG::getInstance()->getDefaultAdapter();
        if(ptr)
        {
            ptr->start();
        }
    }
}

void Daemon::activeUserChange(QString user)
{
    if(CONFIG::getInstance()->active_user() != user)
    {
        CONFIG::getInstance()->active_user(user);

        KyInfo() << "new user: "<< user;

        LingmoAdapterPtr ptr = ADAPTERMNG::getInstance()->getDefaultAdapter();
        if(ptr)
        {
            ptr->set_need_reconnect(true);
            ptr->reconnect_dev();
        }
    }
}


void Daemon::__unblock_sleep(int ms)
{
    QEventLoop eventloop;
    QTimer::singleShot(ms, &eventloop, SLOT(quit()));
    eventloop.exec();
}

void Daemon::wait_for_finish(BluezQt::PendingCall *call)
{
    QEventLoop eventloop;
    connect(call, &BluezQt::PendingCall::finished, this, [&](BluezQt::PendingCall *callReturn)
    {
        eventloop.exit();
    });
    eventloop.exec();
}

