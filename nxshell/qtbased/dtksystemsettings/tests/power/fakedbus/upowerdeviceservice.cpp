// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "upowerdeviceservice.h"

#include <qdbusconnection.h>

DPOWER_USE_NAMESPACE

UPowerDeviceService::UPowerDeviceService(QObject *parent)
    : QObject(parent)
    , m_reset(false)
{
    registerService();
}

UPowerDeviceService::~UPowerDeviceService()
{
    unRegisterService();
}

void UPowerDeviceService::registerService()
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

void UPowerDeviceService::unRegisterService()
{
    QDBusConnection bus = QDBusConnection::sessionBus();
    bus.unregisterObject(QLatin1String("/com/deepin/daemon/FakePower"));
    bus.unregisterService(QLatin1String("com.deepin.daemon.FakePower"));
}

bool UPowerDeviceService::hasHistory() const
{
    return false;
}

bool UPowerDeviceService::hasStatistics() const
{
    return true;
}

bool UPowerDeviceService::isPresent() const
{
    return true;
}

bool UPowerDeviceService::isRechargeable() const
{
    return true;
}

bool UPowerDeviceService::online() const
{
    return true;
}

bool UPowerDeviceService::powerSupply() const
{
    return true;
}

double UPowerDeviceService::capacity() const
{
    return 70;
}

double UPowerDeviceService::energy() const
{
    return 100;
}

double UPowerDeviceService::energyEmpty() const
{
    return 0;
}

double UPowerDeviceService::energyFull() const
{
    return 100;
}

double UPowerDeviceService::energyFullDesign() const
{
    return 33.8;
}

double UPowerDeviceService::energyRate() const
{
    return 99.7;
}

double UPowerDeviceService::luminosity() const
{
    return 10;
}

double UPowerDeviceService::percentage() const
{
    return 70;
}

double UPowerDeviceService::temperature() const
{
    return 33.5;
}

double UPowerDeviceService::voltage() const
{
    return 20.7;
}

qint32 UPowerDeviceService::chargeCycles() const
{
    return 4;
}

qint64 UPowerDeviceService::timeToEmpty() const
{
    return 12345678;
}

qint64 UPowerDeviceService::timeToFull() const
{
    return 12345678;
}

QString UPowerDeviceService::iconName() const
{
    return "full";
}

QString UPowerDeviceService::model() const
{
    return "online";
}

QString UPowerDeviceService::nativePath() const
{
    return "path";
}

QString UPowerDeviceService::serial() const
{
    return "ok";
}

QString UPowerDeviceService::vendor() const
{
    return "ok";
}

quint32 UPowerDeviceService::batteryLevel() const
{
    return 2;
}

quint32 UPowerDeviceService::state() const
{
    return 2;
}

quint32 UPowerDeviceService::technology() const
{
    return 2;
}

quint32 UPowerDeviceService::type() const
{
    return 2;
}

quint32 UPowerDeviceService::warningLevel() const
{
    return 2;
}

quint64 UPowerDeviceService::updateTime() const
{
    return 166443275;
}

QString UPowerDeviceService::deviceName() const
{
    return name;
}

QList<History_p> UPowerDeviceService::GetHistory(const QString &type,
                                                 const uint timespan,
                                                 const uint resolution) const
{
    History_p history;
    history.time = timespan;
    history.value = 100;
    history.state = resolution;
    Q_UNUSED(type)
    QList<History_p> reval;
    reval.append(history);
    history.value = 200;
    reval.append(history);
    return reval;
}

QList<Statistic_p> UPowerDeviceService::GetStatistics(const QString &type) const
{
    Q_UNUSED(type)
    Statistic_p statistic;
    statistic.accuracy = 100;
    statistic.value = 100;
    QList<Statistic_p> reval;
    reval.append(statistic);
    statistic.accuracy = 200;
    statistic.value = 200;
    reval.append(statistic);
    return reval;
}
