// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dservicemanager.h"

#include "dhuangliservice.h"
#include "daccountmanagerservice.h"
#include "units.h"
#include "commondef.h"

#include "dbuscloudsync.h"
#include <QDBusConnection>
#include <QDBusError>


DServiceManager::DServiceManager(QObject *parent)
    : QObject(parent)
{
    //注册服务
    QDBusConnection sessionBus = QDBusConnection::sessionBus();
    if (!sessionBus.registerService(serviceBaseName)) {
        qCritical(ServiceLogger) << "registerService failed:" << sessionBus.lastError();
        exit(0x0001);
    }

    QDBusConnection::RegisterOptions options = QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals | QDBusConnection::ExportAllProperties;
    //创建黄历服务
    DServiceBase *huangliService = new class DHuangliService(this);
    if (!sessionBus.registerObject(huangliService->getPath(), huangliService->getInterface(), huangliService, options)) {
        qCritical(ServiceLogger) << "registerObject huangliService failed:" << sessionBus.lastError();
        exit(0x0002);
    }

    //创建帐户管理服务
    m_accountManagerService = new class DAccountManagerService(this);
    if (!sessionBus.registerObject(m_accountManagerService->getPath(), m_accountManagerService->getInterface(), m_accountManagerService, options)) {
        qCritical(ServiceLogger) << "registerObject  accountManagerService failed:" << sessionBus.lastError();
        exit(0x0003);
    }

    //创建云同步回调服务
    DServiceBase *cloudsyncService = new class Dbuscloudsync(this);
    if (!sessionBus.registerObject(cloudsyncService->getPath(), cloudsyncService->getInterface(), cloudsyncService, options)) {
        qCritical(ServiceLogger) << "registerObject  cloudsyncService failed:" << sessionBus.lastError();
        exit(0x0004);
    }
}

void DServiceManager::updateRemindJob()
{
    if(nullptr != m_accountManagerService){
        m_accountManagerService->updateRemindJob(false);
    }
}
