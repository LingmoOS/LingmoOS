// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "securitylogadaptorimpl.h"
#include "monitornetflow_interface.h"
#include "../../deepin-defender/src/window/modules/common/common.h"

#define DBUS_SERVER "com.deepin.defender.MonitorNetFlow"
#define DBUS_PATH "/com/deepin/defender/MonitorNetFlow"

#define DB_NAME "SecurityLog"
#define DB_TYPE "QSQLITE"

SecurityLogAdaptorImpl::SecurityLogAdaptorImpl(QObject *parent)
    : QObject(parent)
{
    m_monitorInterface = new ComDeepinDefenderMonitorNetFlowInterface(DBUS_SERVER,
                                                                      DBUS_PATH,
                                                                      QDBusConnection::systemBus(), this);

    if (QSqlDatabase::contains(DB_NAME)) {
        m_sqlDb = QSqlDatabase::database(DB_NAME);
    } else {
        m_sqlDb = QSqlDatabase::addDatabase(DB_TYPE, DB_NAME);
    }

    // 加载安全日志记录数据库
    m_sqlDb.setDatabaseName(DEFENDER_LOCAL_CACHE_DB_PATH);
}

QSqlDatabase SecurityLogAdaptorImpl::GetDatabase()
{
    return m_sqlDb;
}

bool SecurityLogAdaptorImpl::DeleteSecurityLog(int date, int type)
{
    if (m_monitorInterface) {
        return m_monitorInterface->DeleteSecurityLog(date, type);
    }

    return false;
}

void SecurityLogAdaptorImpl::AddSecurityLog(int nType, const QString &sInfo)
{
    if (m_monitorInterface) {
        m_monitorInterface->AddSecurityLog(nType, sInfo);
    }
}
