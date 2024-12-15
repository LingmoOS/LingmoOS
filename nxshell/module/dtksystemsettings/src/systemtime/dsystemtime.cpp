// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dsystemtime.h"

#include "dbus/dsystemtimetypes_p.h"
#include "dbus/timedate1interface.h"
#include "dbus/timesync1interface.h"
#include "dsystemtime_p.h"

#include <dexpected.h>
#include <qdatetime.h>
#include <qdbusreply.h>
#include <qdebug.h>
DSYSTEMTIME_BEGIN_NAMESPACE
using DCORE_NAMESPACE::DError;
using DCORE_NAMESPACE::DExpected;
using DCORE_NAMESPACE::DUnexpected;

DSystemTime::DSystemTime(QObject *parent)
    : QObject(parent)
    , d_ptr(new DSystemTimePrivate(this))
{
    Q_D(DSystemTime);
    d->m_timedate_inter = new TimeDate1Interface(this);
    d->m_timesync_inter = new TimeSync1Interface(this);
    // without signals
}

QStringList DSystemTime::fallbackNTPServers() const
{
    Q_D(const DSystemTime);
    return d->m_timesync_inter->fallbackNTPServers();
}

QStringList DSystemTime::linkNTPServers() const
{
    Q_D(const DSystemTime);
    return d->m_timesync_inter->linkNTPServers();
}

QStringList DSystemTime::systemNTPServers() const
{
    Q_D(const DSystemTime);
    return d->m_timesync_inter->systemNTPServers();
}

qint64 DSystemTime::frequency() const
{
    Q_D(const DSystemTime);
    return d->m_timesync_inter->frequency();
}

QString DSystemTime::serverName() const
{
    Q_D(const DSystemTime);
    return d->m_timesync_inter->serverName();
}

Address DSystemTime::serverAddress() const
{
    Q_D(const DSystemTime);
    Address_p address_p = d->m_timesync_inter->serverAddress();
    Address address;
    address.ip = address_p.ip;
    address.type = address_p.type;
    return address;
}

Message DSystemTime::NTPMessage() const
{
    Q_D(const DSystemTime);
    Message_p message_p = d->m_timesync_inter->NTPMessage();
    Message message;
    message.ntp_field_leap = message_p.ntp_field_leap;
    message.ntp_field_version = message_p.ntp_field_version;
    message.ntp_field_mode = message_p.ntp_field_mode;
    message.ntp_msg_stratum = message_p.ntp_msg_stratum;
    message.ntp_msg_precision = message_p.ntp_msg_precision;
    message.ntp_ts_short_to_usec_delay = message_p.ntp_ts_short_to_usec_delay;
    message.ntp_ts_short_to_usec_dispersion = message_p.ntp_ts_short_to_usec_dispersion;
    message.ntp_msg_refid = message_p.ntp_msg_refid;
    message.origin_time = message_p.origin_time;
    message.ntp_msg_recv_time = message_p.ntp_msg_recv_time;
    message.ntp_msg_trans_time = message_p.ntp_msg_trans_time;
    message.dest_time = message_p.dest_time;
    message.spike = message_p.spike;
    message.packet_count = message_p.packet_count;
    message.samples_jitter = message_p.samples_jitter;
    return message;
}

quint64 DSystemTime::pollIntervalMaxUSec() const
{
    Q_D(const DSystemTime);
    return d->m_timesync_inter->pollIntervalMaxUSec();
}

quint64 DSystemTime::pollIntervalMinUSec() const
{
    Q_D(const DSystemTime);
    return d->m_timesync_inter->pollIntervalMinUSec();
}

quint64 DSystemTime::pollIntervalUSec() const
{
    Q_D(const DSystemTime);
    return d->m_timesync_inter->pollIntervalUSec();
}

quint64 DSystemTime::rootDistanceMaxUSec() const
{
    Q_D(const DSystemTime);
    return d->m_timesync_inter->rootDistanceMaxUSec();
}

DSystemTime::~DSystemTime() { }

// properties
bool DSystemTime::canNTP() const
{
    Q_D(const DSystemTime);
    return d->m_timedate_inter->canNTP();
}

bool DSystemTime::localRTC() const
{
    Q_D(const DSystemTime);
    return d->m_timedate_inter->localRTC();
}

bool DSystemTime::NTP() const
{
    Q_D(const DSystemTime);
    return d->m_timedate_inter->NTP();
}

bool DSystemTime::NTPSynchronized() const
{
    Q_D(const DSystemTime);
    return d->m_timedate_inter->NTPSynchronized();
}

QString DSystemTime::timezone() const
{
    Q_D(const DSystemTime);
    return d->m_timedate_inter->timezone();
}

quint64 DSystemTime::RTCTimeUSec() const
{
    Q_D(const DSystemTime);
    return d->m_timedate_inter->RTCTimeUSec();
}

QDateTime DSystemTime::timeDate() const
{
    Q_D(const DSystemTime);
    QDateTime date;
    return date.fromMSecsSinceEpoch(d->m_timedate_inter->timeUSec() / 1000);
}

// slots
DExpected<QStringList> DSystemTime::listTimezones() const
{
    Q_D(const DSystemTime);
    QDBusPendingReply<QStringList> reply = d->m_timedate_inter->listTimezones();
    reply.waitForFinished();
    QStringList timezones;
    if (!reply.isValid()) {
        return DUnexpected{ DError{ reply.error().type(), reply.error().message() } };
    }
    timezones = reply.value();
    return timezones;
}

DExpected<void> DSystemTime::setLocalRTC(bool localRTC, bool fixSystem, bool interactive)
{
    Q_D(DSystemTime);
    QDBusPendingReply<> reply = d->m_timedate_inter->setLocalRTC(localRTC, fixSystem, interactive);
    reply.waitForFinished();
    if (!reply.isValid()) {
        return DUnexpected{ DError{ reply.error().type(), reply.error().message() } };
    }
    return {};
}

DExpected<void> DSystemTime::enableNTP(bool useNTP, bool interactive)
{
    Q_D(DSystemTime);
    QDBusPendingReply<> reply = d->m_timedate_inter->setNTP(useNTP, interactive);
    reply.waitForFinished();
    if (!reply.isValid()) {
        return DUnexpected{ DError{ reply.error().type(), reply.error().message() } };
    }
    return {};
}

DExpected<void> DSystemTime::setRelativeTime(qint64 usecUTC, bool interactive)
{
    Q_D(DSystemTime);
    QDBusPendingReply<> reply = d->m_timedate_inter->setTime(usecUTC, 1, interactive);
    reply.waitForFinished();
    if (!reply.isValid()) {
        return DUnexpected{ DError{ reply.error().type(), reply.error().message() } };
    }
    return {};
}

DExpected<void> DSystemTime::setAbsoluteTime(const QDateTime &time, bool interactive)
{
    Q_D(DSystemTime);
    QDBusPendingReply<> reply =
            d->m_timedate_inter->setTime(time.toMSecsSinceEpoch() * 1000, 0, interactive);
    reply.waitForFinished();
    if (!reply.isValid()) {
        return DUnexpected{ DError{ reply.error().type(), reply.error().message() } };
    }
    return {};
}

DExpected<void> DSystemTime::setTimezone(const QString &timezone, bool interactive)
{
    Q_D(DSystemTime);
    QDBusPendingReply<> reply = d->m_timedate_inter->setTimezone(timezone, interactive);
    reply.waitForFinished();
    if (!reply.isValid()) {
        return DUnexpected{ DError{ reply.error().type(), reply.error().message() } };
    }
    return {};
}

DSYSTEMTIME_END_NAMESPACE
