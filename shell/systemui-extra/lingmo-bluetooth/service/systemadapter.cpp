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

#include "systemadapter.h"
#include "sessiondbusregister.h"
#include "adapter.h"
#include "device.h"
#include "config.h"
#include "app.h"
#include "filesess.h"
#include "common.h"
#include "buriedpointdata.h"
#include "leavelock.h"

SystemAdapter::SystemAdapter(QObject *parent): QObject(parent)
{
    KyDebug();
}

SystemAdapter::~SystemAdapter()
{
    KyDebug();
}

QMap<QString, QVariant> SystemAdapter::registerClient(QMap<QString, QVariant> params)
{
    QString dbusid = this->getCallerDebus();
    KyInfo() << params << " dbusid: "<< dbusid;
    QMap<QString, QVariant> res;
    res["result"] = SYSDBUSMNG::getInstance()->registerClient(params, dbusid);
    res["envPC"] = envPC;
    return res;
}

bool SystemAdapter::unregisterClient(QString id)
{
    KyInfo() << id << " dbusid: "<<this->getCallerDebus();
    return SYSDBUSMNG::getInstance()->unregisterClient(id);
}

QStringList SystemAdapter::getRegisterClient()
{
    KyInfo() << " dbusid: "<<this->getCallerDebus();
    return APPMNG::getInstance()->getRegisterClient();
}

QStringList SystemAdapter::getAllAdapterAddress()
{
    KyInfo() << " dbusid: "<<this->getCallerDebus();
    return ADAPTERMNG::getInstance()->getAllAdapterAddress();
}

QMap<QString, QVariant> SystemAdapter::getAdapterAttr(QString addr, QString key)
{
    KyInfo() << " dbusid: "<<this->getCallerDebus();
    return ADAPTERMNG::getInstance()->getAdapterAttr(addr.toUpper(), key);
}

QStringList SystemAdapter::getDefaultAdapterAllDev()
{
    KyInfo() << " dbusid: "<<this->getCallerDebus();
    return ADAPTERMNG::getInstance()->getDefaultAdapterAllDev();
}

QStringList SystemAdapter::getDefaultAdapterPairedDev()
{
    KyInfo() << " dbusid: "<<this->getCallerDebus();
    return ADAPTERMNG::getInstance()->getDefaultAdapterPairedDev();
}

QMap<QString, QVariant> SystemAdapter::getDevAttr(QString addr)
{
    addr = addr.toUpper();

    KyInfo() << addr  << " dbusid: "<<this->getCallerDebus();
    LingmoAdapterPtr ptr = ADAPTERMNG::getInstance()->getDefaultAdapter();
    LingmoDevicePtr d = nullptr;
    if(ptr)
    {
        d = ptr->getDevPtr(addr);
    }

    if(d)
    {
        return d->getDevAttr();
    }

    return QMap<QString, QVariant>();
}

bool SystemAdapter::setDevAttr(QString dev, QMap<QString, QVariant> attrs)
{
    KyInfo() << dev << attrs  << " dbusid: "<<this->getCallerDebus();
    dev = dev.toUpper();
    KyInfo() << dev << attrs << " dbusid: "<<this->getCallerDebus();
    LingmoAdapterPtr ptr = ADAPTERMNG::getInstance()->getDefaultAdapter();
    LingmoDevicePtr d = nullptr;
    if(ptr)
    {
        d = ptr->getDevPtr(dev);
    }

    if(d)
    {
        d->setDevAttr(attrs);
    }

    return true;
}

bool SystemAdapter::setDefaultAdapterAttr(QMap<QString, QVariant> attrs)
{
    KyInfo() << attrs  << " dbusid: "<<this->getCallerDebus();
    LingmoAdapterPtr ptr = ADAPTERMNG::getInstance()->getDefaultAdapter();
    if(ptr)
    {
        QString key;
        key = AdapterAttr(enum_Adapter_attr_Powered);
        if(attrs.contains(key) && attrs[key].type() == QVariant::Bool)
        {
            bool v = attrs[key].toBool();
            CONFIG::getInstance()->power_switch(v);
            ptr->setPower(v);
        }

        key = AdapterAttr(enum_Adapter_attr_Discoverable);
        if(attrs.contains(key) && attrs[key].type() == QVariant::Bool)
        {
            bool v = attrs[key].toBool();
            CONFIG::getInstance()->discoverable_switch(v);
            ptr->setDiscoverable(v);
        }

        key = AdapterAttr(enum_Adapter_attr_Name);
        if(attrs.contains(key) && attrs[key].type() == QVariant::String)
        {
            QString v = attrs[key].toString();
            ptr->setName(v);
        }

        key = AdapterAttr(enum_Adapter_attr_ActiveConnection);
        if(attrs.contains(key) && attrs[key].type() == QVariant::Bool)
        {
            bool v = attrs[key].toBool();
            CONFIG::getInstance()->activeconnection(v);
            ptr->setActiveConnection(v);
        }

        key = AdapterAttr(enum_Adapter_attr_TrayShow);
        if(attrs.contains(key) && attrs[key].type() == QVariant::Bool)
        {
            bool v = attrs[key].toBool();
            CONFIG::getInstance()->trayShow(v);
            ptr->trayShow(v);
        }

        return true;
    }
    return false;
}

int SystemAdapter::setDefaultAdapter(QString addr)
{
    qInfo() << Q_FUNC_INFO << addr << " dbusid: "<<this->getCallerDebus();
    return ADAPTERMNG::getInstance()->setDefaultAdapter(addr);
}

int SystemAdapter::devConnect(QString addr)
{
    addr = addr.toUpper();

    KyInfo() << addr << " dbusid: "<<this->getCallerDebus();
    int ret = ERR_BREDR_INTERNAL_NO_Default_Adapter;

    LingmoAdapterPtr ptr = ADAPTERMNG::getInstance()->getDefaultAdapter();
    if(ptr)
    {
        ret = ptr->devConnect(addr);
        if(ERR_BREDR_CONN_SUC == ret && ptr->devIsAudioType(addr))
        {
            CONFIG::getInstance()->finally_audiodevice(addr);
        }
    }
    return ret;
}

int SystemAdapter::devDisconnect(QString addr)
{
    addr = addr.toUpper();

    KyInfo() << addr << " dbusid: "<<this->getCallerDebus();
    int ret = ERR_BREDR_INTERNAL_NO_Default_Adapter;
    LingmoAdapterPtr ptr = ADAPTERMNG::getInstance()->getDefaultAdapter();
    if(ptr)
    {
        ret = ptr->devDisconnect(addr);
        if(addr == CONFIG::getInstance()->finally_audiodevice())
        {
            CONFIG::getInstance()->finally_audiodevice("");
        }
    }
    return ret;
}

int SystemAdapter::devRemove(QStringList devlist)
{
    KyInfo() << devlist << " dbusid: "<<this->getCallerDebus();
    LingmoAdapterPtr ptr = ADAPTERMNG::getInstance()->getDefaultAdapter();
    if(ptr)
    {
        ptr->devRemove(devlist);
        for(auto iter : devlist)
        {
            CONFIG::getInstance()->delete_rename(iter.toUpper());
        }
    }
    return 0;
}

int SystemAdapter::activeConnectionReply(QString addr, bool v)
{
    addr = addr.toUpper();
    KyInfo() << addr << v << " dbusid: "<<this->getCallerDebus();
    LingmoAdapterPtr ptr = ADAPTERMNG::getInstance()->getDefaultAdapter();
    if(ptr)
    {
        ptr->activeConnectionReply(addr, v);
    }
    return 0;
}

int SystemAdapter::pairFuncReply(QString addr, bool v)
{
    addr = addr.toUpper();
    KyInfo() << addr << v << " dbusid: "<<this->getCallerDebus();
    LingmoAdapterPtr ptr = ADAPTERMNG::getInstance()->getDefaultAdapter();
    LingmoDevicePtr d = nullptr;
    if(ptr)
    {
        d = ptr->getDevPtr(addr);
    }

    if(d)
    {
        return d->pairFuncReply(v);
    }

    return -1;
}

int SystemAdapter::sendFiles(QString user, QString addr, QStringList files)
{
    addr = addr.toUpper();
    KyInfo() << addr << files << " dbusid: "<<this->getCallerDebus();

    if(HUAWEI == envPC)
    {
        KyInfo() << "not support";
        return -1;
    }

    return FILESESSMNG::getInstance()->sendFiles(addr, files);
}

int SystemAdapter::stopFiles(QMap<QString, QVariant> params)
{
    KyInfo() << params << " dbusid: "<<this->getCallerDebus();
    if(HUAWEI == envPC)
    {
        KyInfo() << "not support";
        return -1;
    }

    QString addr;
    int transportType = enum_filetransport_Type_other;

    QString key;
    key = "dev";
    if(params.contains(key) && params[key].type() == QVariant::String)
    {
        addr = params[key].toString();
        addr = addr.toUpper();
    }

    key = "transportType";
    if(params.contains(key) && (params[key].type() == QVariant::Int || params[key].type() == QVariant::UInt))
    {
        transportType = params[key].toInt();
    }

    if(addr.isEmpty() || enum_filetransport_Type_other == transportType)
    {
        qWarning() << Q_FUNC_INFO << "error";
        return -1;
    }

    return FILESESSMNG::getInstance()->stopFiles(addr, transportType);
}

int SystemAdapter::replyFileReceiving(QMap<QString, QVariant> params)
{
    KyInfo() << params << " dbusid: "<<this->getCallerDebus();
    if(HUAWEI == envPC)
    {
        KyInfo() << "not support";
        return -1;
    }

    QString addr;
    bool v = false;
    QString savePathdir;
    QString user;

    QString key;
    key = "dev";
    if(params.contains(key) && params[key].type() == QVariant::String)
    {
        addr = params[key].toString();
        addr = addr.toUpper();
    }

    key = "receive";
    if(params.contains(key) && params[key].type() == QVariant::Bool)
    {
        v = params[key].toBool();
    }

    key = "savePathdir";
    if(params.contains(key) && params[key].type() == QVariant::String)
    {
        savePathdir = params[key].toString();
    }

    key = "user";
    if(params.contains(key) && params[key].type() == QVariant::String)
    {
        user = params[key].toString();
    }

    if(addr.isEmpty())
    {
        qWarning() << "addr error";
        return -1;
    }

    return FILESESSMNG::getInstance()->replyFileReceiving(addr, v, savePathdir);
}

void SystemAdapter::writeBuriedPointData(QString pluginName, QString settingsName, QString action, QString value)
{
    KyInfo() << pluginName << settingsName << action << value << " dbusid: "<<this->getCallerDebus();
    emit BPDMNG::getInstance()->writeInData(pluginName, settingsName, action, value);
}

QString SystemAdapter::bluetoothKeyValue(unsigned int key, QString str)
{
    KyInfo() << key << str << " dbusid: "<<this->getCallerDebus();
    return SYSDBUSMNG::getInstance()->bluetoothKeyValue(key, str);
}


QString SystemAdapter::getCallerDebus()
{
    QDBusConnection conn = connection();
    QDBusMessage msg = message();
    QString dbusid = conn.interface()->serviceOwner(msg.service()).value();

    if(APPMNG::getInstance()->existDbusid(dbusid))
    {
        return dbusid;
    }

    int uid = conn.interface()->serviceUid(msg.service()).value();
    int64_t pid = conn.interface()->servicePid(msg.service()).value();

    KyInfo() << "unknown dbusid: "<< dbusid << " uid: "<< uid << " pid: " << pid;

    return dbusid;
}

bool SystemAdapter::setLeaveLock(QString uuid, QString dev, bool on)
{
    return LEAVELOCK::getInstance()->setPing(uuid, dev, on);
}
