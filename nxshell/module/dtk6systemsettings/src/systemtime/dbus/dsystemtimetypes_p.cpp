// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "dsystemtimetypes_p.h"

#include <qdbusmetatype.h>

DSYSTEMTIME_BEGIN_NAMESPACE

bool Address_p::operator==(const Address_p &a)
{
    return this->ip == a.ip && this->type == a.type;
}

bool Address_p::operator!=(const Address_p &a)
{
    return this->ip != a.ip || this->type != a.type;
}

QDBusArgument &operator<<(QDBusArgument &arg, const Address_p &address)
{
    arg.beginStructure();
    arg << address.ip;
    arg << address.type;
    arg.endStructure();
    return arg;
}

const QDBusArgument &operator>>(const QDBusArgument &arg, Address_p &address)
{
    arg.beginStructure();
    arg >> address.ip;
    arg >> address.type;
    arg.endStructure();
    return arg;
}

QDBusArgument &operator<<(QDBusArgument &arg, const Message_p &message)
{
    arg.beginStructure();
    arg << message.ntp_field_leap;
    arg << message.ntp_field_version;
    arg << message.ntp_field_mode;
    arg << message.ntp_msg_stratum;
    arg << message.ntp_msg_precision;
    arg << message.ntp_ts_short_to_usec_delay;
    arg << message.ntp_ts_short_to_usec_dispersion;
    arg << message.ntp_msg_refid;
    arg << message.origin_time;
    arg << message.ntp_msg_recv_time;
    arg << message.ntp_msg_trans_time;
    arg << message.dest_time;
    arg << message.spike;
    arg << message.packet_count;
    arg << message.samples_jitter;
    arg.endStructure();
    return arg;
}

const QDBusArgument &operator>>(const QDBusArgument &arg, Message_p &message)
{
    arg.beginStructure();
    arg >> message.ntp_field_leap;
    arg >> message.ntp_field_version;
    arg >> message.ntp_field_mode;
    arg >> message.ntp_msg_stratum;
    arg >> message.ntp_msg_precision;
    arg >> message.ntp_ts_short_to_usec_delay;
    arg >> message.ntp_ts_short_to_usec_dispersion;
    arg >> message.ntp_msg_refid;
    arg >> message.origin_time;
    arg >> message.ntp_msg_recv_time;
    arg >> message.ntp_msg_trans_time;
    arg >> message.dest_time;
    arg >> message.spike;
    arg >> message.packet_count;
    arg >> message.samples_jitter;
    arg.endStructure();
    return arg;
}

void Address_p::registerMetaType()
{
    qRegisterMetaType<Address_p>("Address_p");
    qDBusRegisterMetaType<Address_p>();
}

void Message_p::registerMetaType()
{
    qRegisterMetaType<Message_p>("Message_p");
    qDBusRegisterMetaType<Message_p>();
}

DSYSTEMTIME_END_NAMESPACE