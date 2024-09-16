// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "timedate1service.h"

#include <qdebug.h>

TimeDate1Service::TimeDate1Service(QObject *parent)
{
    Q_UNUSED(parent);
    registerService();
}

TimeDate1Service::~TimeDate1Service()
{
    unRegisterService();
}

void TimeDate1Service::registerService()
{
    const QString &service = QLatin1String("com.deepin.daemon.FakeTimeDate1");
    const QString &path = QLatin1String("/com/deepin/daemon/FakeTimeDate1");
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

void TimeDate1Service::unRegisterService()
{
    QDBusConnection bus = QDBusConnection::sessionBus();
    bus.unregisterObject(QLatin1String("/com/deepin/daemon/FakeTimeDate1"));
    bus.unregisterService(QLatin1String("com.deepin.daemon.FakeTimeDate1"));
}

bool TimeDate1Service::canNTP() const
{
    return true;
}

bool TimeDate1Service::localRTC() const
{
    return true;
}

bool TimeDate1Service::NTP() const
{
    return true;
}

bool TimeDate1Service::NTPSynchronized() const
{
    return true;
}

QString TimeDate1Service::timezone() const
{
    return "Asia/Tokyo";
}

quint64 TimeDate1Service::RTCTimeUSec() const
{
    return 42;
}

quint64 TimeDate1Service::timeUSec() const
{
    return 1666171692;
}

QStringList TimeDate1Service::ListTimezones() const
{
    return { "Asia/Tokyo", "Asia/Shanghai", "Asia/Hongkong", "Asia/Korea" };
}

void TimeDate1Service::SetLocalRTC(const bool local_rtc,
                                   const bool fix_system,
                                   const bool interactive)
{
    setLocalRTC_sig = local_rtc;
    Q_UNUSED(fix_system);
    Q_UNUSED(interactive);
    return;
}

void TimeDate1Service::SetNTP(const bool use_NTP, const bool interactive)
{
    setLocalTime_sig = use_NTP;
    Q_UNUSED(interactive);
    return;
}

void TimeDate1Service::SetTime(const qint64 usec_utc, const bool relative, const bool interactive)
{
    setTime_sig = usec_utc;
    Q_UNUSED(relative);
    Q_UNUSED(interactive);
    return;
}

void TimeDate1Service::SetTimezone(const QString &timezone, const bool interactive)
{
    setTimezone_sig = timezone;
    Q_UNUSED(interactive);
    Q_UNUSED(timezone);
    return;
}