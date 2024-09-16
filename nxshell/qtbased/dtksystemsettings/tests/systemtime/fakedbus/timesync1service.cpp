// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "timesync1service.h"

#include <qdebug.h>

DSYSTEMTIME_BEGIN_NAMESPACE
TimeSync1Service::TimeSync1Service(QObject *parent)
{
    Q_UNUSED(parent);
    Address_p::registerMetaType();
    Message_p::registerMetaType();
    registerService();
}

TimeSync1Service::~TimeSync1Service()
{
    unRegisterService();
}

void TimeSync1Service::registerService()
{
    const QString &service = QLatin1String("com.deepin.daemon.FakeTimeSync1");
    const QString &path = QLatin1String("/com/deepin/daemon/FakeTimeSync1");
    QDBusConnection bus = QDBusConnection::sessionBus();
    // register
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

void TimeSync1Service::unRegisterService()
{
    QDBusConnection bus = QDBusConnection::sessionBus();
    bus.unregisterObject(QLatin1String("/com/deepin/daemon/FakeTimeSync1"));
    bus.unregisterService(QLatin1String("com.deepin.daemon.FakeTimeSync1"));
}

QStringList TimeSync1Service::fallbackNTPServers() const
{
    return QStringList() << "NTP1"
                         << "NTP2";
}

QStringList TimeSync1Service::linkNTPServers() const
{
    return QStringList() << "NTP1Server"
                         << "NTP2Server";
}

QStringList TimeSync1Service::systemNTPServers() const
{
    return QStringList() << "NTP1Server"
                         << "NTP2Server";
}

qint64 TimeSync1Service::frequency() const
{
    return 60;
}

QString TimeSync1Service::serverName() const
{
    return "NTP1";
}

Address_p TimeSync1Service::serverAddress() const
{
    Address_p address;
    address.ip = "127.0.0.1";
    address.type = 1;
    return address;
}

Message_p TimeSync1Service::NTPMessage() const
{
    Message_p message;
    message.ntp_field_leap = 1;
    message.ntp_field_version = 4;
    message.ntp_field_mode = 3;
    message.ntp_msg_stratum = 2;
    message.ntp_msg_precision = 4;
    message.ntp_ts_short_to_usec_delay = 1122334455667788;
    message.ntp_ts_short_to_usec_dispersion = 8877665544332211;
    message.ntp_msg_refid = "qwert";
    message.origin_time = 1234567890;
    message.ntp_msg_recv_time = 987654321;
    message.ntp_msg_trans_time = 87654321;
    message.dest_time = 12345678;
    message.spike = true;
    message.packet_count = 1234567812345678;
    message.samples_jitter = 8765432187654321;

    return message;
}

quint64 TimeSync1Service::pollIntervalMaxUSec() const
{
    return 2048;
}

quint64 TimeSync1Service::pollIntervalMinUSec() const
{
    return 60;
}

quint64 TimeSync1Service::pollIntervalUSec() const
{
    return 240;
}

quint64 TimeSync1Service::rootDistanceMaxUSec() const
{
    return 5;
}

DSYSTEMTIME_END_NAMESPACE
