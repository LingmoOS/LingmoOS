// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
// 
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "dbusproxybase.hpp"
#include "system_org_deepin_dde_Accounts1.h"

#include "system/org_deepin_dde_Accounts1_User.hpp"

class SystemAccounts1Proxy : public DBusProxyBase {
public:
    SystemAccounts1Proxy(QString dbusName, QString dbusPath, QString dbusInterface,
        QString proxyDbusName, QString proxyDbusPath, QString proxyDbusInterface,
        QDBusConnection::BusType dbusType, QObject *parent = nullptr) 
        : DBusProxyBase(dbusName, dbusPath, dbusInterface, proxyDbusName, proxyDbusPath, proxyDbusInterface, dbusType, parent)
    {
        ServiceStart();
    }
    virtual DDBusExtendedAbstractInterface *initConnect()
    {
        m_dbusProxy = new org::deepin::dde::Accounts1(m_dbusName, m_dbusPath, QDBusConnection::systemBus(), this);
        return m_dbusProxy;
    }
    virtual void signalMonitorCustom()
    {
        QString subPathInterface  = "org.deepin.dde.Accounts1.User";
        QString subPathProxyInterface;
        QString subPathProxyPathPrefix;
        if (m_proxyDbusName == "com.deepin.daemon.Accounts") {
            // V0 -> V1子PATH处理
            subPathProxyInterface = "com.deepin.daemon.Accounts.User";
            subPathProxyPathPrefix = "/com/deepin/daemon/Accounts/";
        } else if (m_proxyDbusName == "org.deepin.daemon.Accounts1") {
            // V0.5 -> V1子PATH处理
            subPathProxyInterface = "org.deepin.daemon.Accounts1.User";
            subPathProxyPathPrefix = "/org/deepin/daemon/Accounts1/";
        }
        SubPathInit("UserList", DBusProxySubPathInfo{
                subPathProxyPathPrefix,
                subPathProxyInterface,
                subPathInterface},
            [=](QString path, QString interface, QString proxyPath, QString proxyInterface){
                return new SystemAccounts1UserProxy(m_dbusName, path, interface,
                    m_proxyDbusName, proxyPath, proxyInterface, m_dbusType);
            }
        );
    }
private:
    org::deepin::dde::Accounts1 *m_dbusProxy;
};


