// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
// 
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "dbusproxybase.hpp"
#include "com_deepin_daemon_Accounts.h"

#include "system/com_deepin_daemon_Accounts_User.hpp"

class SystemAccountsProxy : public DBusProxyBase {
   
public:
    SystemAccountsProxy(QString dbusName, QString dbusPath, QString dbusInterface,
        QString proxyDbusName, QString proxyDbusPath, QString proxyDbusInterface,
        QDBusConnection::BusType dbusType, QObject *parent = nullptr)
        : DBusProxyBase(dbusName, dbusPath, dbusInterface, proxyDbusName, proxyDbusPath, proxyDbusInterface, dbusType, parent)
    {
        InitFilterProperies(QStringList({}));
        InitFilterMethods(QStringList({"EnablePasswdChangedHandler", "FindUserByName"}));
        ServiceStart();
    }
    virtual DDBusExtendedAbstractInterface *initConnect()
    {
        m_dbusProxy = new com::deepin::daemon::Accounts(m_dbusName, m_dbusPath, QDBusConnection::systemBus(), this);
        return m_dbusProxy;
        return nullptr;
    }
    virtual void signalMonitorCustom()
    {
        SubPathInit("UserList", [=](QString path){
            qInfo() << "create accounts path proxy";
            QString suffix = path.right(path.size() - (path.lastIndexOf("/") + 1));
            return new SystemAccountsUserProxy(m_dbusName,
                path,
                "com.deepin.daemon.Accounts.User",
                m_proxyDbusName,
                "/com/test/daemon/Accounts/"+suffix,
                "com.test.daemon.Accounts.User",
                m_dbusType);
        });
    }
private:
    com::deepin::daemon::Accounts *m_dbusProxy;
};


