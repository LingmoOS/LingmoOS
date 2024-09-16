// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "daemonpowerservice.h"

#include <qdbusconnection.h>
#include <qdbuserror.h>
#include <qdebug.h>

DaemonPowerService::DaemonPowerService(QObject *parent)
    : QObject(parent)
    , m_reset(false)
    , m_batteryLidClosedAction(0)
    , m_batteryLockDelay(0)
    , m_batteryPressPowerBtnAction(0)
    , m_batteryScreenBlackDelay(0)
    , m_batteryScreensaverDelay(0)
    , m_batterySleepDelay(0)
    , m_linePowerLidClosedAction(0)
    , m_linePowerLockDelay(0)
    , m_linePowerPressPowerBtnAction(0)
    , m_linePowerScreenBlackDelay(0)
    , m_linePowerScreensaverDelay(0)
    , m_linePowerSleepDelay(0)
    , m_lowPowerAutoSleepThreshold(0)
    , m_lowPowerNotifyEnable(false)
    , m_lowPowerNotifyThreshold(0)
    , m_screenBlackLock(false)
    , m_sleepLock(false)
{
    registerService();
}

DaemonPowerService::~DaemonPowerService()
{
    unRegisterService();
}

void DaemonPowerService::registerService()
{
    const QString &service = QLatin1String("com.deepin.daemon.FakePower");
    const QString &path = QLatin1String("/com/deepin/daemon/FakePower");
    QDBusConnection bus = QDBusConnection::sessionBus();
    if (!bus.registerService(service)) {
        QString errorMsg = bus.lastError().message();
        if (errorMsg.isEmpty())
            errorMsg = "maybe it's running";

        qWarning() << QString("Can't register the %1 service, %2.").arg(service).arg(errorMsg);
    }
    if (!bus.registerObject(path, this, QDBusConnection::ExportScriptableContents)) {
        qWarning() << QString("Can't register %1 the D-Bus object.").arg(path);
    }
}

void DaemonPowerService::unRegisterService()
{
    QDBusConnection bus = QDBusConnection::sessionBus();
    bus.unregisterObject(QLatin1String("/com/deepin/daemon/FakePower"));
    bus.unregisterService(QLatin1String("com.deepin.daemon.FakePower"));
}
