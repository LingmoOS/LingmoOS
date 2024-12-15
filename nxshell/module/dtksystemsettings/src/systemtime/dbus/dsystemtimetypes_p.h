// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef DSYSTEMTIMETYPES_P_H
#define DSYSTEMTIMETYPES_P_H

#include "dtksystemtime_global.h"

#include <qdbusargument.h>
#include <qdbusextratypes.h>
#include <qdbusmetatype.h>
#include <qnamespace.h>

DSYSTEMTIME_BEGIN_NAMESPACE

struct Address_p
{
    qint32 type;
    QByteArray ip;

    static void registerMetaType();
    bool operator==(const Address_p &a);
    bool operator!=(const Address_p &a);
};

struct Message_p
{
    quint32 ntp_field_leap;
    quint32 ntp_field_version;
    quint32 ntp_field_mode;
    quint32 ntp_msg_stratum;
    qint32 ntp_msg_precision;
    quint64 ntp_ts_short_to_usec_delay;
    quint64 ntp_ts_short_to_usec_dispersion;
    QByteArray ntp_msg_refid;
    quint64 origin_time;
    quint64 ntp_msg_recv_time;
    quint64 ntp_msg_trans_time;
    quint64 dest_time;
    bool spike;
    quint64 packet_count;
    quint64 samples_jitter;

    static void registerMetaType();
};

QDBusArgument &operator<<(QDBusArgument &arg, const Address_p &address);
const QDBusArgument &operator>>(const QDBusArgument &arg, Address_p &address);

QDBusArgument &operator<<(QDBusArgument &arg, const Message_p &message);
const QDBusArgument &operator>>(const QDBusArgument &arg, Message_p &message);

DSYSTEMTIME_END_NAMESPACE

Q_DECLARE_METATYPE(DTK_SYSTEMTIME_NAMESPACE::Address_p)
Q_DECLARE_METATYPE(DTK_SYSTEMTIME_NAMESPACE::Message_p)

#endif // __DTIMETYPES_H__
