// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: 2011 Craig Drummond <craig.p.drummond@gmail.com>
// SPDX-FileCopyrightText: 2018 Alexis Lopes Zubeta <contact@azubieta.net>
// SPDX-FileCopyrightText: 2020 Tomaz Canabrava <tcanabrava@kde.org>
/*
 * UFW KControl Module
 */

#include "profile.h"
#include "firewallclient.h"

#include <QBuffer>
#include <QFile>
#include <QIODevice>
#include <QStringList>
#include <QTextStream>
#include <QXmlStreamReader>

Profile::Profile(QByteArray &xml, bool isSys)
    : m_fields(0)
    , m_enabled(false)
    , m_ipv6Enabled(false)
    , m_logLevel(Types::LOG_OFF)
    , m_defaultIncomingPolicy(Types::POLICY_ALLOW)
    , m_defaultOutgoingPolicy(Types::POLICY_ALLOW)
    , m_isSystem(isSys)
{
    QBuffer buffer;
    buffer.setBuffer(&xml);
    load(&buffer);
}

Profile::Profile(QFile &file, bool isSys)
    : m_fields(0)
    , m_enabled(false)
    , m_ipv6Enabled(false)
    , m_logLevel(Types::LOG_OFF)
    , m_defaultIncomingPolicy(Types::POLICY_ALLOW)
    , m_defaultOutgoingPolicy(Types::POLICY_ALLOW)
    , m_fileName(file.fileName())
    , m_isSystem(isSys)
{
    load(&file);
}

Profile::Profile(const QList<Rule *> &rules, const QVariantMap &args, bool isSys)
    : m_fields(0)
    , m_enabled(false)
    , m_ipv6Enabled(false)
    , m_logLevel(Types::LOG_OFF)
    , m_defaultIncomingPolicy(Types::POLICY_ALLOW)
    , m_defaultOutgoingPolicy(Types::POLICY_ALLOW)
    , m_isSystem(isSys)
{
    setRules(rules);
    setArgs(args);
}

void Profile::setRules(const QList<Rule *> &newrules)
{
    m_rules = newrules;
}

void Profile::setArgs(const QVariantMap &args)
{
    const QString new_defaultIncomingPolicy = args.value(QStringLiteral("defaultIncomingPolicy")).toString();
    const QString new_defaultOutgoingPolicy = args.value(QStringLiteral("defaultIncomingPolicy")).toString();
    const QString new_loglevel = args.value(QStringLiteral("logLevel")).toString();
    const QStringList new_modules = args.value(QStringLiteral("modules")).toStringList();

    m_defaultIncomingPolicy = new_defaultIncomingPolicy.isEmpty() ? Types::POLICY_ALLOW : Types::toPolicy(new_defaultIncomingPolicy);
    m_defaultOutgoingPolicy = new_defaultOutgoingPolicy.isEmpty() ? Types::POLICY_ALLOW : Types::toPolicy(new_defaultOutgoingPolicy);
    m_logLevel = new_loglevel.isEmpty() ? Types::LOG_OFF : Types::toLogLevel(new_loglevel);
    m_enabled = args.value(QStringLiteral("status")).toBool();
    m_ipv6Enabled = args.value("ipv6Enabled").toBool();

    if (!new_modules.isEmpty()) {
        m_modules = QSet<QString>(std::begin(new_modules), std::end(new_modules));
    }
}

void Profile::setDefaultIncomingPolicy(const QString &policy)
{
    m_defaultIncomingPolicy = Types::toPolicy(policy);
}

void Profile::setDefaultOutgoingPolicy(const QString &policy)
{
    m_defaultOutgoingPolicy = Types::toPolicy(policy);
}

QString Profile::toXml() const
{
    QString str;
    QTextStream stream(&str);

    stream << "<ufw full=\"true\" >" << Qt::endl << ' ' << defaultsXml() << Qt::endl << " <rules>" << Qt::endl;

    /* for (const auto &rule : m_rules) { */
    /* stream << "  " << rule->toXml(); */
    /* } */

    stream << " </rules>" << Qt::endl << ' ' << modulesXml() << Qt::endl << "</ufw>" << Qt::endl;

    return str;
}

QString Profile::defaultsXml() const
{
    static const auto defaultString = QStringLiteral(R"(<defaults ipv6="%1" loglevel="%2" incoming="%3" outgoing="%4"/>)");

    return defaultString.arg(m_ipv6Enabled ? "yes" : "no")
        .arg(Types::toString(m_logLevel))
        .arg(Types::toString(m_defaultIncomingPolicy))
        .arg(Types::toString(m_defaultOutgoingPolicy));
}

QString Profile::modulesXml() const
{
    return QString("<modules enabled=\"") + QStringList(m_modules.values()).join(" ") + QString("\" />");
}

void Profile::load(QIODevice *device)
{
    device->open(QIODevice::ReadOnly);
    QXmlStreamReader reader(device);

    bool isFullProfile = false;
    while (!reader.atEnd()) {
        auto token = reader.readNext();
        if (token == QXmlStreamReader::Invalid) {
            break;
        }
        if (token != QXmlStreamReader::StartElement) {
            continue;
        }
        if (reader.name() == QStringLiteral("ufw")) {
            isFullProfile = reader.attributes().value(QStringLiteral("full")) == QStringLiteral("true");
            continue;
        } else if (reader.name() == QStringLiteral("status")) {
            m_enabled = reader.attributes().value(QStringLiteral("enabled")) == QStringLiteral("true");
            m_fields |= FIELD_STATUS;
        } else if (reader.name() == QStringLiteral("rules")) {
            m_fields |= FIELD_RULES;
            continue;
        } else if (reader.name() == QLatin1String("rule")) {
            static QString ANY_ADDR = QStringLiteral("0.0.0.0/0");
            static QString ANY_ADDR_V6 = QStringLiteral("::/0");
            static QString ANY_PORT = QStringLiteral("any");

            const auto attr = reader.attributes();

            // Handle Enums.
            const auto action = Types::toPolicy(attr.value(QLatin1String("action")).toString());

            // TODO: Check that this is ok.
            const auto protocol = FirewallClient::indexOfProtocol(attr.value(QLatin1String("protocol")).toString());
            /* qDebug() << "Reading protocol from profile:" << protocol << attr.value(QLatin1String("protocol")); */

            const auto logType = Types::toLogging(attr.value(QLatin1String("logtype")).toString());

            // Handle values that have weird defaults.
            const auto anyAddrs = QList<QString>({ANY_ADDR, ANY_ADDR_V6});
            const auto dst = attr.value("dst").toString();
            const auto src = attr.value("src").toString();
            const auto sport = attr.value("sport").toString();
            const auto dport = attr.value("dport").toString();

            const QString destAddress = anyAddrs.contains(dst) ? QString() : dst;
            const QString sourceAddress = anyAddrs.contains(src) ? QString() : src;
            const QString sourcePort = sport == ANY_PORT ? QString() : sport;
            const QString destPort = dport == ANY_PORT ? QString() : dport;

            m_rules.append(new Rule(action,
                                    attr.value("direction") == QStringLiteral("in"),
                                    logType,
                                    protocol,
                                    sourceAddress,
                                    sourcePort,
                                    destAddress,
                                    destPort,
                                    attr.value("interface_in").toString(),
                                    attr.value("interface_out").toString(),
                                    attr.value("sapp").toString(),
                                    attr.value("dapp").toString(),
                                    attr.value("position").toInt(),
                                    attr.value("v6") == QStringLiteral("True")));
        } else if (reader.name() == QLatin1String("defaults")) {
            m_fields |= FIELD_DEFAULTS;

            const auto attr = reader.attributes();

            m_logLevel = Types::toLogLevel(attr.value(QLatin1String("loglevel")).toString());

            m_defaultIncomingPolicy = Types::toPolicy(attr.value(QLatin1String("incoming")).toString());
            m_defaultOutgoingPolicy = Types::toPolicy(attr.value(QLatin1String("outgoing")).toString());

            m_ipv6Enabled = (attr.value("ipv6") == QLatin1String("yes"));
        } else if (reader.name() == QLatin1String("modules")) {
            m_fields |= FIELD_MODULES;
            const auto attr = reader.attributes();
            const auto moduleList = attr.value("enabled").toString().split(" ", Qt::SkipEmptyParts);
            m_modules = QSet<QString>(std::begin(moduleList), std::end(moduleList));
        }
    }
    if (isFullProfile && (!(m_fields & FIELD_RULES) || !(m_fields & FIELD_DEFAULTS) || !(m_fields & FIELD_MODULES))) {
        m_fields = 0;
    }
}

void Profile::setEnabled(bool value)
{
    m_enabled = value;
}
