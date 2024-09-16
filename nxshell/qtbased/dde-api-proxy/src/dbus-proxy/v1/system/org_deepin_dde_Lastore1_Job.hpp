// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "dbusproxybase.hpp"
#include "system_org_deepin_dde_Lastore1_Job.h"

class SystemLastore1JobProxy : public DBusProxyBase {
public:
    SystemLastore1JobProxy(QString dbusName, QString dbusPath, QString dbusInterface,
                           QString proxyDbusName, QString proxyDbusPath, QString proxyDbusInterface,
                           QDBusConnection::BusType dbusType, QObject *parent = nullptr)
        : DBusProxyBase(dbusName, dbusPath, dbusInterface, proxyDbusName, proxyDbusPath, proxyDbusInterface, dbusType, parent)
    {
        ServiceStart();
    }
    virtual DDBusExtendedAbstractInterface *initConnect()
    {
        m_dbusProxy = new org::deepin::dde::lastore1::Job(m_dbusName, m_dbusPath, QDBusConnection::systemBus(), this);
        return m_dbusProxy;
    }
private:
    org::deepin::dde::lastore1::Job *m_dbusProxy;
};
