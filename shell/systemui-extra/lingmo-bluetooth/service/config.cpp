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

#include "config.h"
#include <QDBusContext>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusReply>

Environment envPC = Environment::NOMAL;

#define CONF_FILE_FULLPATH                      "/etc/bluetooth/lingmo-bluetooth.conf"

#define CONF_STR_BLUETOOTH_POWER_SWTICH          "switch"
#define CONF_STR_BLUETOOTH_DISCOVERABLE_SWTICH   "bluetoothDiscoverableSwitch"
#define CONF_STR_FINALLY_CONNECT_THE_DEVICE      "finallyConnectTheDevice"
#define CONF_STR_ADAPTER_ADDRESS                 "adapterAddress"
#define CONF_STR_FILE_SAVE_PATH                  "fileSavePath"
#define CONF_STR_ADAPTER_ADDRESS_LIST            "adapterAddressList"
#define CONF_STR_ACTIVE_CONNECTION               "activeConnection"
#define CONF_STR_TRAY_SHOW                       "trayShow"

#define CONF_LOG_LEVEL                           "loglevel"


Config::Config(QObject *parent)
    :QObject(parent)
{
    m_lingmo_bluetooth = new QSettings(CONF_FILE_FULLPATH, QSettings::IniFormat);
    QStringList groups = m_lingmo_bluetooth->childGroups();
    KyInfo() << groups;
    if(groups.indexOf("conf") == -1)
    {
        KyInfo() << "create and init conf";
        m_lingmo_bluetooth->beginGroup("conf");
        this->init_conf();
    }
    else
    {
        m_lingmo_bluetooth->beginGroup("conf");
        this->init_conf();
    }
}

Config::~Config()
{
    delete m_lingmo_bluetooth;
    m_lingmo_bluetooth = nullptr;
}

void Config::init_conf()
{
    if(!m_lingmo_bluetooth->contains(CONF_STR_BLUETOOTH_POWER_SWTICH))
    {
        KyInfo() << "add conf: " << CONF_STR_BLUETOOTH_POWER_SWTICH;
        m_lingmo_bluetooth->setValue(CONF_STR_BLUETOOTH_POWER_SWTICH, false);
    }

    if(!m_lingmo_bluetooth->contains(CONF_STR_BLUETOOTH_DISCOVERABLE_SWTICH))
    {
        KyInfo() << "add conf: " << CONF_STR_BLUETOOTH_DISCOVERABLE_SWTICH;
        m_lingmo_bluetooth->setValue(CONF_STR_BLUETOOTH_DISCOVERABLE_SWTICH, true);
    }

    if(!m_lingmo_bluetooth->contains(CONF_STR_FINALLY_CONNECT_THE_DEVICE))
    {
        KyInfo() << "add conf: " << CONF_STR_FINALLY_CONNECT_THE_DEVICE;
        m_lingmo_bluetooth->setValue(CONF_STR_FINALLY_CONNECT_THE_DEVICE, "");
    }

    if(!m_lingmo_bluetooth->contains(CONF_STR_ADAPTER_ADDRESS))
    {
        KyInfo() << "add conf: " << CONF_STR_ADAPTER_ADDRESS;
        m_lingmo_bluetooth->setValue(CONF_STR_ADAPTER_ADDRESS, "");
    }

    if(!m_lingmo_bluetooth->contains(CONF_STR_FILE_SAVE_PATH))
    {
        KyInfo() << "add conf: " << CONF_STR_FILE_SAVE_PATH;
        m_lingmo_bluetooth->setValue(CONF_STR_FILE_SAVE_PATH, "");
    }

    if(!m_lingmo_bluetooth->contains(CONF_STR_ADAPTER_ADDRESS_LIST))
    {
        KyInfo() << "add conf: " << CONF_STR_ADAPTER_ADDRESS_LIST;
        QStringList t;
        t.append("null");
        m_lingmo_bluetooth->setValue(CONF_STR_ADAPTER_ADDRESS_LIST, t);
    }

    if(!m_lingmo_bluetooth->contains(CONF_STR_ACTIVE_CONNECTION))
    {
        KyInfo() << "add conf: " << CONF_STR_ACTIVE_CONNECTION;
        m_lingmo_bluetooth->setValue(CONF_STR_ACTIVE_CONNECTION, true);
    }

    if(!m_lingmo_bluetooth->contains(CONF_STR_TRAY_SHOW))
    {
        KyInfo() << "add conf: " << CONF_STR_TRAY_SHOW;
        m_lingmo_bluetooth->setValue(CONF_STR_TRAY_SHOW, true);
    }
}

int Config::init()
{
    if(m_lingmo_bluetooth->contains(CONF_STR_BLUETOOTH_POWER_SWTICH))
    {
        m_power_switch = m_lingmo_bluetooth->value(CONF_STR_BLUETOOTH_POWER_SWTICH).toBool();
        KyInfo() << CONF_STR_BLUETOOTH_POWER_SWTICH << m_power_switch;
    }

    if(m_lingmo_bluetooth->contains(CONF_STR_BLUETOOTH_DISCOVERABLE_SWTICH))
    {
        m_discoverable_switch = m_lingmo_bluetooth->value(CONF_STR_BLUETOOTH_DISCOVERABLE_SWTICH).toBool();
        KyInfo() << CONF_STR_BLUETOOTH_DISCOVERABLE_SWTICH << m_discoverable_switch;
    }

    if(m_lingmo_bluetooth->contains(CONF_STR_FINALLY_CONNECT_THE_DEVICE))
    {
        m_finally_audiodevice = m_lingmo_bluetooth->value(CONF_STR_FINALLY_CONNECT_THE_DEVICE).toString();
        KyInfo() << CONF_STR_FINALLY_CONNECT_THE_DEVICE << m_finally_audiodevice;
    }

    if(m_lingmo_bluetooth->contains(CONF_STR_ADAPTER_ADDRESS))
    {
        m_adapter_addr = m_lingmo_bluetooth->value(CONF_STR_ADAPTER_ADDRESS).toString();
        KyInfo() << CONF_STR_ADAPTER_ADDRESS << m_adapter_addr;
    }

    if(m_lingmo_bluetooth->contains(CONF_STR_FILE_SAVE_PATH))
    {
        m_file_save_path = m_lingmo_bluetooth->value(CONF_STR_FILE_SAVE_PATH).toString();
        KyInfo() << CONF_STR_FILE_SAVE_PATH << m_file_save_path;
    }

    if(m_lingmo_bluetooth->contains(CONF_STR_ACTIVE_CONNECTION))
    {
        m_activeconnection = m_lingmo_bluetooth->value(CONF_STR_ACTIVE_CONNECTION).toBool();
        KyInfo() << CONF_STR_ACTIVE_CONNECTION << m_activeconnection;
    }

    if(m_lingmo_bluetooth->contains(CONF_STR_TRAY_SHOW))
    {
        m_tray_show = m_lingmo_bluetooth->value(CONF_STR_TRAY_SHOW).toBool();
        KyInfo() << CONF_STR_TRAY_SHOW << m_tray_show;
    }

    if(m_lingmo_bluetooth->contains(CONF_LOG_LEVEL))
    {
        m_loglevel = m_lingmo_bluetooth->value(CONF_LOG_LEVEL).toString();
        KyInfo() << CONF_LOG_LEVEL << m_loglevel;
    }

    m_active_user = this->getActiveUser();
    KyInfo()<< "active user: "<< m_active_user;

    //m_file_watch = new QFileSystemWatcher(QStringList(CONF_FILE_FULLPATH));
    //connect(m_file_watch, &QFileSystemWatcher::fileChanged, this, &Config::fileChanged);
    //KyInfo() << "monistor: "<< m_file_watch->files();

    this->init_rename();
    return 0;
}

void Config::power_switch(bool v)
{
    m_power_switch = v;
    m_lingmo_bluetooth->setValue(CONF_STR_BLUETOOTH_POWER_SWTICH, v);
}

void Config::discoverable_switch(bool v)
{
    m_discoverable_switch = v;
    m_lingmo_bluetooth->setValue(CONF_STR_BLUETOOTH_DISCOVERABLE_SWTICH, v);
}

void Config::finally_audiodevice(QString v)
{
    m_finally_audiodevice = v;
    m_lingmo_bluetooth->setValue(CONF_STR_FINALLY_CONNECT_THE_DEVICE, v);
}

void Config::adapter_addr(QString v)
{
    m_adapter_addr = v;
    m_lingmo_bluetooth->setValue(CONF_STR_ADAPTER_ADDRESS, v);
}

void Config::file_save_path(QString v)
{
    m_file_save_path = v;
    m_lingmo_bluetooth->setValue(CONF_STR_FILE_SAVE_PATH, v);
}

void Config::activeconnection(bool v)
{
    m_activeconnection = v;
    m_lingmo_bluetooth->setValue(CONF_STR_ACTIVE_CONNECTION, v);
}

void Config::trayShow(bool v)
{
    m_tray_show = v;
    m_lingmo_bluetooth->setValue(CONF_STR_TRAY_SHOW, v);
}

void Config::loglevel(QString v)
{
    m_loglevel = v;
    m_lingmo_bluetooth->setValue(CONF_LOG_LEVEL, v);
}

void Config::set_rename(QString addr, QString name)
{
    m_rename[addr] = name;
    QSettings s(CONF_FILE_FULLPATH, QSettings::IniFormat);
    s.beginGroup("rename");
    s.setValue(addr, name);
    s.endGroup();
}

QString Config::get_rename(QString v)
{
    if(m_rename.contains(v))
    {
        return m_rename[v];
    }
    return "";
}

void Config::delete_rename(QString addr)
{
    if(m_rename.contains(addr))
    {
        QSettings s(CONF_FILE_FULLPATH, QSettings::IniFormat);
        s.beginGroup("rename");
        s.remove(addr);
        s.endGroup();
        m_rename.remove(addr);
    }
}


void Config::init_rename()
{
    QSettings s(CONF_FILE_FULLPATH, QSettings::IniFormat);
    s.beginGroup("rename");
    QStringList keys = s.allKeys();
    for(auto iter : keys)
    {
        m_rename[iter] = s.value(iter).toString();
    }
    KyDebug() << m_rename;
    s.endGroup();
}




QString Config::getActiveUser()
{
    QString user;
    QDBusInterface iface("org.lingmo.test",
                         "/com/lingmo/test",
                         "com.lingmo.test",
                         QDBusConnection::systemBus());

    QDBusPendingCall pcall = iface.asyncCall("GetActiveUser");
    pcall.waitForFinished();

    QDBusMessage res = pcall.reply();
    if(res.type() == QDBusMessage::ReplyMessage)
    {
        if(res.arguments().size() > 0)
        {
            user = res.arguments().takeFirst().toString();
        }
    }
    else
    {
        qWarning()<< res.errorName() << ": "<< res.errorMessage();
    }
    return user;
}









