// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: 2011 Craig Drummond <craig.p.drummond@gmail.com>
// SPDX-FileCopyrightText: 2018 Alexis Lopes Zubeta <contact@azubieta.net>
// SPDX-FileCopyrightText: 2020 Tomaz Canabrava <tcanabrava@kde.org>

#ifndef UFW_PROFILE_H
#define UFW_PROFILE_H

/*
 * UFW KControl Module
 */

#include <kcm_firewall_core_export.h>

#include <QByteArray>
#include <QList>
#include <QSet>

#include "rule.h"
#include "types.h"

class QFile;
class QIODevice;

class KCM_FIREWALL_CORE_EXPORT Profile
{
public:
    enum Fields { FIELD_RULES = 0x01, FIELD_DEFAULTS = 0x02, FIELD_MODULES = 0x04, FIELD_STATUS = 0x08 };

    explicit Profile(QByteArray &xml, bool isSys = false);
    explicit Profile(QFile &file, bool isSys = false);

    Profile()
        : m_fields(0)
        , m_enabled(false)
        , m_ipv6Enabled(false)
        , m_logLevel(Types::LOG_OFF)
        , m_defaultIncomingPolicy(Types::POLICY_ALLOW)
        , m_defaultOutgoingPolicy(Types::POLICY_ALLOW)
        , m_rules({})
        , m_modules({})
        , m_isSystem(false)
    {
    }

    Profile(const QList<Rule *> &rules, const QVariantMap &args, bool isSys = false);
    Profile(bool ipv6, Types::LogLevel ll, Types::Policy dip, Types::Policy dop, const QList<Rule *> &r, const QSet<QString> &m)
        : m_fields(0xFF)
        , m_enabled(true)
        , m_ipv6Enabled(ipv6)
        , m_logLevel(ll)
        , m_defaultIncomingPolicy(dip)
        , m_defaultOutgoingPolicy(dop)
        , m_rules(r)
        , m_modules(m)
        , m_isSystem(false)
    {
    }

    bool operator==(const Profile &o) const
    {
        return m_ipv6Enabled == o.m_ipv6Enabled && m_logLevel == o.m_logLevel && m_defaultIncomingPolicy == o.m_defaultIncomingPolicy
            && m_defaultOutgoingPolicy == o.m_defaultOutgoingPolicy && m_rules == o.m_rules && m_modules == o.m_modules;
    }

    QString toXml() const;
    QString defaultsXml() const;
    QString modulesXml() const;

    bool hasRules() const
    {
        return m_fields & FIELD_RULES;
    }
    bool hasDefaults() const
    {
        return m_fields & FIELD_DEFAULTS;
    }
    bool hasModules() const
    {
        return m_fields & FIELD_MODULES;
    }
    bool hasStatus() const
    {
        return m_fields & FIELD_STATUS;
    }

    int fields() const
    {
        return m_fields;
    }
    bool enabled() const
    {
        return m_enabled;
    }
    bool ipv6Enabled() const
    {
        return m_ipv6Enabled;
    }
    Types::LogLevel logLevel() const
    {
        return m_logLevel;
    }
    Types::Policy defaultIncomingPolicy() const
    {
        return m_defaultIncomingPolicy;
    }
    Types::Policy defaultOutgoingPolicy() const
    {
        return m_defaultOutgoingPolicy;
    }
    const QList<Rule *> rules() const
    {
        return m_rules;
    }
    const QSet<QString> modules() const
    {
        return m_modules;
    }
    const QString fileName() const
    {
        return m_fileName;
    }
    bool isSystem() const
    {
        return m_isSystem;
    }

    void setRules(const QList<Rule *> &newrules);
    void setArgs(const QVariantMap &args);
    void setEnabled(bool value);
    void setDefaultIncomingPolicy(const QString &policy);
    void setDefaultOutgoingPolicy(const QString &policy);

private:
    void load(QIODevice *device);

private:
    int m_fields;
    bool m_enabled, m_ipv6Enabled;
    Types::LogLevel m_logLevel;
    Types::Policy m_defaultIncomingPolicy, m_defaultOutgoingPolicy;
    QList<Rule *> m_rules;
    QSet<QString> m_modules;
    QString m_fileName;
    bool m_isSystem;
};

#endif
