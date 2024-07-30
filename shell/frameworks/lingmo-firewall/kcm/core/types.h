// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: 2011 Craig Drummond <craig.p.drummond@gmail.com>
// SPDX-FileCopyrightText: 2018 Alexis Lopes Zubeta <contact@azubieta.net>
// SPDX-FileCopyrightText: 2020 Tomaz Canabrava <tcanabrava@kde.org>

#ifndef UFW_TYPES_H
#define UFW_TYPES_H

/*
 * UFW KControl Module
 */

#include <kcm_firewall_core_export.h>

#include <QString>
#include <QVariant>

namespace Types
{
KCM_FIREWALL_CORE_EXPORT Q_NAMESPACE

    enum LogLevel {
        LOG_OFF,
        LOG_LOW,
        LOG_MEDIUM,
        LOG_HIGH,
        LOG_FULL,

        LOG_COUNT
    };
Q_ENUM_NS(LogLevel)

enum Logging {
    LOGGING_OFF,
    LOGGING_NEW,
    LOGGING_ALL,

    LOGGING_COUNT
};
Q_ENUM_NS(Logging)

enum Policy {
    POLICY_ALLOW,
    POLICY_DENY,
    POLICY_REJECT,
    POLICY_LIMIT,

    POLICY_COUNT,
    POLICY_COUNT_DEFAULT = POLICY_COUNT - 1 // No 'Limit' for defaults...
};
Q_ENUM_NS(Policy)

enum PredefinedPort {
    PP_AMULE,
    PP_DELUGE,
    PP_KTORRENT,
    PP_NICOTINE,
    PP_QBITTORRNET,
    PP_TRANSMISSION,
    PP_IM_ICQ,
    PP_IM_JABBER,
    PP_IM_WLM,
    PP_IM_YAHOO,

    PP_FTP,
    PP_HTTP,
    PP_HTTPS,
    PP_IMAP,
    PP_IMAPS,
    PP_POP3,
    PP_POP3S,
    PP_SMTP,
    PP_NFS,
    PP_SAMBA,
    PP_SSH,
    PP_VNC,
    PP_ZEROCONF,
    PP_TELNET,
    PP_NTP,
    PP_CUPS,

    PP_COUNT
};
Q_ENUM_NS(PredefinedPort)

KCM_FIREWALL_CORE_EXPORT QString toString(LogLevel level, bool ui = false);
KCM_FIREWALL_CORE_EXPORT LogLevel toLogLevel(const QString &str);
KCM_FIREWALL_CORE_EXPORT QString toString(Logging log, bool ui = false);
KCM_FIREWALL_CORE_EXPORT Logging toLogging(const QString &str);
KCM_FIREWALL_CORE_EXPORT QString toString(Policy policy, bool ui = false);
KCM_FIREWALL_CORE_EXPORT Policy toPolicy(const QString &str);
KCM_FIREWALL_CORE_EXPORT QString toString(PredefinedPort pp, bool ui = false);
KCM_FIREWALL_CORE_EXPORT PredefinedPort toPredefinedPort(const QString &str);

}

#endif
