// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
// 
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "dbusproxybase.hpp"
#include "session_org_deepin_dde_Display1.h"

#include "session/org_deepin_dde_Display1_Monitor.hpp"

class SessionDisplay1Proxy : public DBusProxyBase {
public:
    SessionDisplay1Proxy(QString dbusName, QString dbusPath, QString dbusInterface,
        QString proxyDbusName, QString proxyDbusPath, QString proxyDbusInterface,
        QDBusConnection::BusType dbusType, QObject *parent = nullptr)
        : DBusProxyBase(dbusName, dbusPath, dbusInterface, proxyDbusName, proxyDbusPath, proxyDbusInterface, dbusType, parent)
    {
        // InitFilterProperies(QStringList({"Brightnes", "Monitors", "Primary", "ScreenWidth", "ScreenHeight", "PrimaryRect"}));
        // InitFilterMethods(QStringList({"SetBrightness"}));
        ServiceStart();
    }
    virtual DDBusExtendedAbstractInterface *initConnect()
    {
        m_dbusProxy = new org::deepin::dde::Display1(m_dbusName, m_dbusPath, QDBusConnection::sessionBus(), this);
        return m_dbusProxy;
    }
    virtual void signalMonitorCustom()
    {
        SubPathInit("Monitors", DBusProxySubPathInfo{
                "/com/deepin/daemon/Display/",
                "com.deepin.daemon.Display.Monitor",
                "org.deepin.dde.Display1.Monitor"},
            [=](QString path, QString interface, QString proxyPath, QString proxyInterface){
                return new SessionDisplay1MonitorProxy(m_dbusName, path, interface,
                    m_proxyDbusName, proxyPath, proxyInterface, m_dbusType);
            }
        );
    }
private:
    org::deepin::dde::Display1 *m_dbusProxy;
};


