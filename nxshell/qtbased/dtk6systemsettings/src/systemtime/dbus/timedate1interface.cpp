// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "timedate1interface.h"

#include <qdbusconnection.h>
#include <qdbusextratypes.h>
#include <qstring.h>
#include <qvariant.h>

DSYSTEMTIME_BEGIN_NAMESPACE
TimeDate1Interface::TimeDate1Interface(QObject *parent)
    : QObject(parent)
{
#ifdef USE_FAKE_INTERFACE
    static const QString &Service = QStringLiteral("com.deepin.daemon.FakeTimeDate1");
    static const QString &Path = QStringLiteral("/com/deepin/daemon/FakeTimeDate1");
    static const QString &Interface = QStringLiteral("com.deepin.daemon.FakeTimeDate1");
    QDBusConnection connection = QDBusConnection::sessionBus();
#else
    static const QString &Service = QStringLiteral("org.freedesktop.timedate1");
    static const QString &Interface = QStringLiteral("org.freedesktop.timedate1");
    static const QString &Path = "/org/freedesktop/timedate1";
    QDBusConnection connection = QDBusConnection::systemBus();
#endif
    m_inter = new DDBusInterface(Service, Path, Interface, connection, this);
}

TimeDate1Interface::~TimeDate1Interface(){};

bool TimeDate1Interface::canNTP() const
{
    return qdbus_cast<bool>(m_inter->property("CanNTP"));
}

bool TimeDate1Interface::localRTC() const
{
    return qdbus_cast<bool>(m_inter->property("LocalRTC"));
}

bool TimeDate1Interface::NTP() const
{
    return qdbus_cast<bool>(m_inter->property("NTP"));
}

bool TimeDate1Interface::NTPSynchronized() const
{
    return qdbus_cast<bool>(m_inter->property("NTPSynchronized"));
}

QString TimeDate1Interface::timezone() const
{
    return qdbus_cast<QString>(m_inter->property("Timezone"));
}

quint64 TimeDate1Interface::RTCTimeUSec() const
{
    return qdbus_cast<quint64>(m_inter->property("RTCTimeUSec"));
}

quint64 TimeDate1Interface::timeUSec() const
{
    return qdbus_cast<quint64>(m_inter->property("TimeUSec"));
}

QDBusPendingReply<QStringList> TimeDate1Interface::listTimezones() const
{
    QDBusPendingReply<> reply = m_inter->asyncCall(QLatin1String("ListTimezones"));
    return reply;
};

QDBusPendingReply<> TimeDate1Interface::setLocalRTC(const bool local_rtc,
                                                    const bool fix_system,
                                                    const bool interactive)
{
    QDBusPendingReply<> reply =
            m_inter->asyncCallWithArgumentList(QLatin1String("SetLocalRTC"),
                                               { local_rtc, fix_system, interactive });
    return reply;
};

QDBusPendingReply<> TimeDate1Interface::setNTP(const bool use_NTP, const bool interactive)
{
    QDBusPendingReply<> reply =
            m_inter->asyncCallWithArgumentList(QLatin1String("SetNTP"), { use_NTP, interactive });
    return reply;
};

QDBusPendingReply<> TimeDate1Interface::setTime(const qint64 usec_utc,
                                                const bool relative,
                                                const bool interactive)
{
    QDBusPendingReply<> reply =
            m_inter->asyncCallWithArgumentList(QLatin1String("SetTime"),
                                               { usec_utc, relative, interactive });
    return reply;
};

QDBusPendingReply<> TimeDate1Interface::setTimezone(const QString &timezone, const bool interactive)
{
    QDBusPendingReply<> reply =
            m_inter->asyncCallWithArgumentList(QLatin1String("SetTimezone"),
                                               { QVariant::fromValue(timezone), interactive });
    return reply;
};

DSYSTEMTIME_END_NAMESPACE