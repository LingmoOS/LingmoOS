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

#include "app.h"
#include <QDBusConnection>

#include "adapter.h"
#include "common.h"
#include "config.h"


Application::Application(QString dbusid, QString username, int type)
{
    m_dbusid = dbusid;
    m_username = username;
    m_type = type;
    KyInfo() << "dbusid: " << m_dbusid <<", username: " << m_username
           << ", type: " << m_type;
}

Application::~Application()
{
    KyInfo() << "dbusid: " << m_dbusid;
}


ApplicationMng::ApplicationMng()
{
    m_watch = new QDBusServiceWatcher("",QDBusConnection::systemBus(),
                                QDBusServiceWatcher::WatchForUnregistration, nullptr);

    connect(m_watch, &QDBusServiceWatcher::serviceUnregistered, this, &ApplicationMng::serviceUnregistered);
}

ApplicationMng::~ApplicationMng()
{
    m_watch->disconnect(this);
    m_watch->deleteLater();
}

int ApplicationMng::add(QString dbusid, QString username, int type)
{
    KyInfo() << dbusid << username << type;
    if(m_apps.contains(dbusid))
    {
        return 0;
    }
    else
    {
        Application * mem = new Application(dbusid, username, type);
        m_apps[dbusid] = mem;
        m_watch->addWatchedService(dbusid);
    }

    this->update();

    return 1;
}

int ApplicationMng::remove(QString dbusid)
{
    KyInfo() << dbusid;
    if(m_apps.contains(dbusid))
    {
        delete m_apps[dbusid];
        m_apps.remove(dbusid);

        this->update();
    }
    return 1;
}

int ApplicationMng::getControlNum()
{
    int num = 0;
    for(auto item : m_apps.toStdMap())
    {
        //控制面板
        if(item.second->type() == enum_app_type_bluetooth_controlPanel)
        {
            //仅计算活跃用户的控制面板数量
            if(item.second->username() == CONFIG::getInstance()->active_user())
            {
                num++;
            }
        }
    }
    return num;
}

bool ApplicationMng::existDbusid(QString id)
{
    return m_apps.contains(id);
}

QStringList ApplicationMng::getRegisterClient()
{
    QStringList apps;
    for(auto item : m_apps.toStdMap())
    {
        apps.append(item.first);
    }
    return apps;
}

QString ApplicationMng::getUserDbusid(QString user, int type)
{
    for(auto item : m_apps.toStdMap())
    {
        //控制面板
        if(item.second->type() == type && item.second->username() == user)
        {
            return item.second->dbusid();
        }
    }
    return "";
}

void ApplicationMng::update()
{
    LingmoAdapterPtr ptr = ADAPTERMNG::getInstance()->getDefaultAdapter();
    if(ptr)
    {
        ptr->updateDiscovering();
    }
}

void ApplicationMng::serviceUnregistered(const QString &service)
{
    KyInfo() << service;
    this->remove(service);
}

