// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
// 
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "dbusproxybase.hpp"
#include "session_org_deepin_dde_daemon_Dock1.h"

#include "session/org_deepin_dde_daemon_Dock1_Entry.hpp"

class SessionDaemonDock1Proxy : public DBusProxyBase {
public:
    SessionDaemonDock1Proxy(QString dbusName, QString dbusPath, QString dbusInterface,
        QString proxyDbusName, QString proxyDbusPath, QString proxyDbusInterface,
        QDBusConnection::BusType dbusType, QObject *parent = nullptr)
        : DBusProxyBase(dbusName, dbusPath, dbusInterface, proxyDbusName, proxyDbusPath, proxyDbusInterface, dbusType, parent)
    {
        // InitFilterProperies(QStringList({"FrontendWindowRect", "HideState", "HideMode", "DisplayMode", "Position"}));
        // InitFilterMethods(QStringList({"RequestDock", "RequestUndock", "ActivateWindow"}));
        ServiceStart();
    }
    virtual DDBusExtendedAbstractInterface *initConnect()
    {
        m_dbusProxy = new org::deepin::dde::daemon::Dock1(m_dbusName, m_dbusPath, QDBusConnection::sessionBus(), this);
        return m_dbusProxy;
    }
    virtual void signalMonitorCustom()
    {
        SubPathInit("Entries", DBusProxySubPathInfo{
                "/com/deepin/dde/daemon/Dock/entries/",
                "com.deepin.dde.daemon.Dock.Entry",
                "org.deepin.dde.daemon.Dock1.Entry"},
            [=](QString path, QString interface, QString proxyPath, QString proxyInterface){
                return new SessionDaemonDock1EntryProxy(m_dbusName, path, interface,
                    m_proxyDbusName, proxyPath, proxyInterface, m_dbusType);
            }
        );
    }
private:
    org::deepin::dde::daemon::Dock1 *m_dbusProxy;
};


