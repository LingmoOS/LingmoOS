// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DSYSTEMTIMETYPES_H
#define DSYSTEMTIMETYPES_H

#include "dtksystemtime_global.h"

#include <qobject.h>

DSYSTEMTIME_BEGIN_NAMESPACE

struct Address
{
    qint32 type;
    QByteArray ip;
    friend bool operator==(const Address &lhs, const Address &rhs)
    {
        return lhs.type == rhs.type && lhs.ip == rhs.ip;
    }
};

struct Message
{
    quint32 ntp_field_leap; /*!< @~english warning code Indicates the next leap to be inserted in the NTP time scale. A value of
                               "11" indicates an alarm state and the clock cannot be synchronized.*/
    quint32 ntp_field_version; /*!< @~english Version number of NTP*/
    quint32 ntp_field_mode; /*!< @~english The operating modes of NTP. The meanings of the different values are as follows: 0:
                               reserved,    reserved. 1: symmetric active, active peer mode. 2:
                               symmetric passive, passive peer mode. 3:    client, client mode. 4:
                               server, server mode. 5: broadcast, broadcast mode. 6: reserved for
                               NTP control messages, NTP control messages. 7: reserved for private
                               use, NTP control messages. 8: reserved for private use, NTP control
                               messages. 9: reserved for private use, NTP control messages. reserved
                               for NTP control messages. 7: reserved for private use, reserved for
                               internal use.*/
    quint32 ntp_msg_stratum; /*!< @~english The number of layers of the clock defines the accuracy of the clock. The clock with
                                layer number   1 has the highest accuracy, decreasing from 1
                                to 15.*/
    qint32 ntp_msg_precision; /*!< @~english Accuracy of the clock*/
    quint64 ntp_ts_short_to_usec_delay; /*!< @~english Total round-trip delay time to the master reference clock*/
    quint64 ntp_ts_short_to_usec_dispersion; /*< The maximum error of the local clock with respect
                                                to the main reference clock.*/
    QByteArray ntp_msg_refid; /*< The NTP REFID specifies a string of 4 ASCII characters that
                                 defines the reference identifier used by the driver.   The REFID
                                 string overrides the default identifier ordinarily assigned by the
                                 driver itself.*/
    quint64 origin_time; /*!< @~english The local time when the NTP message leaves the source. */
    quint64 ntp_msg_recv_time; /*!< @~english Local time of arrival of NTP messages at the destination*/
    quint64 ntp_msg_trans_time; /*!< @~english The local time when the answer message leaves the server side at the destination*/
    quint64 dest_time; /*!< @~english The time to reply to the NTP server after the NTP message is received locally*/
    bool spike;
    quint64 packet_count;
    quint64 samples_jitter;
    friend bool operator==(const Message &lhs, const Message &rhs)
    {
        return lhs.ntp_field_leap == rhs.ntp_field_leap &&
               lhs.ntp_field_version == rhs.ntp_field_version &&
               lhs.ntp_field_mode == rhs.ntp_field_mode &&
               lhs.ntp_msg_stratum == rhs.ntp_msg_stratum &&
               lhs.ntp_msg_precision == rhs.ntp_msg_precision &&
               lhs.ntp_ts_short_to_usec_delay == rhs.ntp_ts_short_to_usec_delay &&
               lhs.ntp_ts_short_to_usec_dispersion == rhs.ntp_ts_short_to_usec_dispersion &&
               lhs.ntp_msg_refid == rhs.ntp_msg_refid &&
               lhs.origin_time == rhs.origin_time &&
               lhs.ntp_msg_recv_time == rhs.ntp_msg_recv_time &&
               lhs.ntp_msg_trans_time == rhs.ntp_msg_trans_time &&
               lhs.dest_time == rhs.dest_time &&
               lhs.spike == rhs.spike &&
               lhs.packet_count == rhs.packet_count &&
               lhs.samples_jitter == rhs.samples_jitter;
    }
    
};

DSYSTEMTIME_END_NAMESPACE

#endif
