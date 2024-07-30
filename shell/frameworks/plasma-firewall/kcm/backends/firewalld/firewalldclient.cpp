// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: 2020 Lucas Biaggi <lbjanuario@gmail.com>

#include <KConfigGroup>
#include <KLocalizedString>
#include <KPluginFactory>

#include <QDBusMetaType>
#include <QDebug>
#include <QDir>
#include <QNetworkInterface>
#include <QProcess>
#include <QStandardPaths>
#include <QVariantList>
#include <QVariantMap>

#include <loglistmodel.h>
#include <rulelistmodel.h>
#include <systemdjob.h>

#include "firewalldclient.h"
#include "firewalldjob.h"
#include "firewalldlogmodel.h"
#include "queryrulesfirewalldjob.h"

#include "dbustypes.h"

K_PLUGIN_CLASS_WITH_JSON(FirewalldClient, "firewalldbackend.json")
Q_LOGGING_CATEGORY(FirewallDClientDebug, "firewalld.client")

FirewalldClient::FirewalldClient(QObject *parent, const QVariantList &args)
    : IFirewallClientBackend(parent, args)
    , m_rulesModel(new RuleListModel(this))
    , m_logsAutoRefresh(false)
{
    queryExecutable("firewalld");

    qDBusRegisterMetaType<firewalld_reply>();
    qDBusRegisterMetaType<QList<firewalld_reply>>();
}

QString FirewalldClient::name() const
{
    return QStringLiteral("firewalld");
}

void FirewalldClient::refresh()
{
    queryStatus(FirewallClient::DefaultDataBehavior::ReadDefaults, FirewallClient::ProfilesBehavior::ListenProfiles);
}

bool FirewalldClient::enabled() const
{
    return m_currentProfile.enabled();
}

KJob *FirewalldClient::setEnabled(const bool value)
{
    if (enabled() == value) {
        return nullptr;
    }

    SystemdJob *job = new SystemdJob(static_cast<SYSTEMD::actions>(value), QStringLiteral("firewalld.service"));

    connect(job, &KJob::result, this, [this, job, value] {
        if (job->error()) {
            qCDebug(FirewallDClientDebug) << "Job Error: " << job->error() << job->errorString();
            return;
        }
        m_currentProfile.setEnabled(value);
        if (value) {
            FirewalldJob *authjob = new FirewalldJob(FirewalldJob::ALL);
            connect(authjob, &KJob::result, this, [this, authjob] {
                if (authjob->error()) {
                    qCDebug(FirewallDClientDebug) << "Job AuthError: " << authjob->error() << authjob->errorString();
                    return;
                }
                queryStatus(FirewallClient::DefaultDataBehavior::ReadDefaults, FirewallClient::ProfilesBehavior::DontListenProfiles);
            });
            authjob->start();
        }
        Q_EMIT enabledChanged(value);
    });

    return job;
}

KJob *FirewalldClient::queryStatus(FirewallClient::DefaultDataBehavior defaultsBehavior, FirewallClient::ProfilesBehavior profilesBehavior)
{
    Q_UNUSED(defaultsBehavior);
    Q_UNUSED(profilesBehavior);
    QueryRulesFirewalldJob *job = new QueryRulesFirewalldJob();

    connect(job, &QueryRulesFirewalldJob::result, this, [this, job] {
        if (job->error()) {
            qCDebug(FirewallDClientDebug) << "Query rules job error: " << job->error() << job->errorString();
            return;
        }
        qCDebug(FirewallDClientDebug) << job->name();
        QList<Rule *> const rules = extractRulesFromResponse(job->getFirewalldreply()) + extractRulesFromResponse(job->getServices());
        const QVariantMap args = {{"defaultIncomingPolicy", defaultIncomingPolicy()},
                                  {"defaultOutgoingPolicy", defaultOutgoingPolicy()},
                                  {"status", true},
                                  {"ipv6Enabled", true}};
        setProfile(Profile(rules, args));
    });
    job->start();
    return job;
}

void FirewalldClient::setLogsAutoRefresh(bool logsAutoRefresh)
{
    if (m_logsAutoRefresh == logsAutoRefresh) {
        return;
    }

    if (logsAutoRefresh) {
        connect(&m_logsRefreshTimer, &QTimer::timeout, this, &FirewalldClient::refreshLogs);
        m_logsRefreshTimer.setInterval(3000);
        m_logsRefreshTimer.start();
    } else {
        disconnect(&m_logsRefreshTimer, &QTimer::timeout, this, &FirewalldClient::refreshLogs);
        m_logsRefreshTimer.stop();
    }

    m_logsAutoRefresh = logsAutoRefresh;
    Q_EMIT logsAutoRefreshChanged(m_logsAutoRefresh);
}

void FirewalldClient::refreshLogs(){};

RuleListModel *FirewalldClient::rules() const
{
    return m_rulesModel;
}

Rule *FirewalldClient::ruleAt(int index)
{
    auto cRules = m_currentProfile.rules();

    if (index < 0 || index >= cRules.count()) {
        return nullptr;
    }

    Rule *rule = cRules.at(index);
    return rule;
}
KJob *FirewalldClient::addRule(Rule *rule)
{
    if (rule == nullptr) {
        qWarning() << "Invalid rule";
        return nullptr;
    }

    qCDebug(FirewallDClientDebug) << rule->toStr();

    QVariantList dbusArgs = buildRule(rule);
    if (rule->simplified()) {
        dbusArgs.push_back(QVariant(0));
    }
    qCDebug(FirewallDClientDebug) << "sending job ... rule simplified ? " << rule->simplified();
    qCDebug(FirewallDClientDebug) << "Dbus Args...." << dbusArgs;
    FirewalldJob *job = rule->simplified() ? new FirewalldJob("addService", dbusArgs, FirewalldJob::SIMPLIFIEDRULE) : new FirewalldJob("addRule", dbusArgs);

    connect(job, &KJob::result, this, [this, job] {
        if (job->error()) {
            qCDebug(FirewallDClientDebug) << job->errorString() << job->error();
            return;
        }
        queryStatus(FirewallClient::DefaultDataBehavior::ReadDefaults, FirewallClient::ProfilesBehavior::DontListenProfiles);
    });

    job->start();
    return job;
}

KJob *FirewalldClient::removeRule(int index)
{
    QVariantList dbusArgs = buildRule(ruleAt(index));
    // FirewalldJob *job = new FirewalldJob("removeRule", dbusArgs);
    FirewalldJob *job = ruleAt(index)->simplified() ? new FirewalldJob("removeService", dbusArgs, FirewalldJob::SIMPLIFIEDRULE) : // if true simplie interface
        new FirewalldJob("removeRule", dbusArgs);
    connect(job, &KJob::result, this, [this, job] {
        if (job->error()) {
            qCDebug(FirewallDClientDebug) << job->errorString() << job->error();
            return;
        }
        refresh();
    });

    job->start();
    return job;
}

KJob *FirewalldClient::updateRule(Rule *ruleWrapper)
{
    /* not supported by the backend */
    // TODO
    Q_UNUSED(ruleWrapper)
    return nullptr;
}

bool FirewalldClient::supportsRuleUpdate() const
{
    return false;
}

KJob *FirewalldClient::moveRule(int from, int to)
{
    QList<Rule *> cRules = m_currentProfile.rules();
    if (from < 0 || from >= cRules.count()) {
        qWarning() << "invalid from index";
    }

    if (to < 0 || to >= cRules.count()) {
        qWarning() << "invalid to index";
    }
    // Correct indices
    from += 1;
    to += 1;

    QVariantMap args{
        {"cmd", "moveRule"},
        {"from", from},
        {"to", to},
    };

    return new FirewalldJob();
}

bool FirewalldClient::logsAutoRefresh() const
{
    return m_logsAutoRefresh;
}

Rule *FirewalldClient::createRuleFromConnection(const QString &protocol, const QString &localAddress, const QString &foreignAddres, const QString &status)
{
    auto _localAddress = localAddress;
    _localAddress.replace("*", "");
    _localAddress.replace("0.0.0.0", "");

    auto _foreignAddres = foreignAddres;
    _foreignAddres.replace("*", "");
    _foreignAddres.replace("0.0.0.0", "");

    auto localAddressData = _localAddress.split(":");
    auto foreignAddresData = _foreignAddres.split(":");

    auto rule = new Rule();
    rule->setIncoming(status == QStringLiteral("LISTEN"));
    rule->setPolicy("deny");

    // Prepare rule draft
    if (status == QStringLiteral("LISTEN")) {
        rule->setSourceAddress(foreignAddresData[0]);
        rule->setSourcePort(foreignAddresData[1]);
        rule->setDestinationAddress(localAddressData[0]);
        rule->setDestinationPort(localAddressData[1]);
    } else {
        rule->setSourceAddress(localAddressData[0]);
        rule->setSourcePort(localAddressData[1]);
        rule->setDestinationAddress(foreignAddresData[0]);
        rule->setDestinationPort(foreignAddresData[1]);
    }

    rule->setProtocol(knownProtocols().indexOf(protocol.toUpper()));
    return rule;
}

Rule *FirewalldClient::createRuleFromLog(const QString &protocol,
                                         const QString &sourceAddress,
                                         const QString &sourcePort,
                                         const QString &destinationAddress,
                                         const QString &destinationPort,
                                         const QString &inn)
{
    // Transform to the ufw notation
    auto rule = new Rule();

    auto _sourceAddress = sourceAddress;
    _sourceAddress.replace("*", "");
    _sourceAddress.replace("0.0.0.0", "");

    auto _destinationAddress = destinationAddress;
    _destinationAddress.replace("*", "");
    _destinationAddress.replace("0.0.0.0", "");

    // Prepare rule draft
    rule->setIncoming(inn.size());
    rule->setPolicy("allow");
    rule->setSourceAddress(_sourceAddress);
    rule->setSourcePort(sourcePort);

    rule->setDestinationAddress(_destinationAddress);
    rule->setDestinationPort(destinationPort);

    rule->setProtocol(knownProtocols().indexOf(protocol.toUpper()));
    return rule;
}

void FirewalldClient::refreshProfiles()
{
}

bool FirewalldClient::isTcpAndUdp(int protocolIdx)
{
    Q_UNUSED(protocolIdx);
    return false;
}

QVariantList FirewalldClient::buildRule(const Rule *r) const
{
    qCDebug(FirewallDClientDebug) << "rule simplified? -> " << r->simplified();
    if (r->simplified()) {
        qCDebug(FirewallDClientDebug) << "rule simplified content: " << r->toStr();
        if (!r->sourceApplication().isEmpty()) {
            return QVariantList({"", r->sourceApplication()});
        }
    }
    QVariantMap args{
        {"priority", 0},
        {"destinationPort", r->destinationPort()},
        {"sourcePort", r->sourcePort()},
        {"type", QString(r->protocolSuffix(r->protocol())).replace("/", "")}, // tcp or udp
        {"destinationAddress", r->destinationAddress()},
        {"sourceAddress", r->sourceAddress()},
        {"interface_in", r->interfaceIn()},
        {"interface_out", r->interfaceOut()},
        {"table", "filter"},
    };

    args.insert("chain", r->incoming() ? "INPUT" : "OUTPUT");

    switch (r->action()) {
    case Types::POLICY_ALLOW:
        args.insert("action", "ACCEPT");
        break;
    case Types::POLICY_REJECT:
        args.insert("action", "REJECT");
        break;
    default:
        args.insert("action", "DROP");
    }

    QStringList firewalld_direct_rule = {"-j", args.value("action").toString()};

    auto value = args.value("type").toString();
    if (!value.isEmpty()) {
        firewalld_direct_rule << "-p" << value.toLower();
    }

    value = args.value("destinationAddress").toString();
    if (!value.isEmpty()) {
        firewalld_direct_rule << "-d" << value;
    }

    value = args.value("destinationPort").toString();
    if (!value.isEmpty()) {
        firewalld_direct_rule << "--dport=" + value;
    }

    value = args.value("sourceAddress").toString();
    if (!value.isEmpty()) {
        firewalld_direct_rule << "-s" << value;
    }

    value = args.value("sourcePort").toString();
    if (!value.isEmpty()) {
        firewalld_direct_rule << "--sport=" + value;
    }

    value = args.value(args.value("chain") == "INPUT" ? "interface_in" : "interface_out").toString();
    if (!value.isEmpty() && !value.isNull()) {
        firewalld_direct_rule << "-i" << value;
    }

    QString ipvf = r->ipv6() == true ? "ipv6" : "ipv4";
    qCDebug(FirewallDClientDebug) << firewalld_direct_rule;
    return QVariantList({ipvf, args.value("table"), args.value("chain"), args.value("priority"), firewalld_direct_rule});
}

QString FirewalldClient::defaultIncomingPolicy() const
{
    auto policy_t = m_currentProfile.defaultIncomingPolicy();
    return Types::toString(policy_t);
};

QString FirewalldClient::defaultOutgoingPolicy() const
{
    auto policy_t = m_currentProfile.defaultOutgoingPolicy();
    return Types::toString(policy_t);
};

KJob *FirewalldClient::setDefaultIncomingPolicy(QString defaultIncomingPolicy)
{
    FirewalldJob *job = new FirewalldJob();
    connect(job, &KJob::result, this, [this, job, defaultIncomingPolicy] {
        if (job->error()) {
            qCDebug(FirewallDClientDebug) << job->errorString() << job->error();
            return;
        }
        m_currentProfile.setDefaultIncomingPolicy(defaultIncomingPolicy);
    });

    job->start();
    return job;
};

KJob *FirewalldClient::setDefaultOutgoingPolicy(QString defaultOutgoingPolicy)
{
    FirewalldJob *job = new FirewalldJob();
    connect(job, &KJob::result, this, [this, job, defaultOutgoingPolicy] {
        if (job->error()) {
            qCDebug(FirewallDClientDebug) << job->errorString() << job->error();
            return;
        }
        m_currentProfile.setDefaultOutgoingPolicy(defaultOutgoingPolicy);
    });

    job->start();
    return job;
};

KJob *FirewalldClient::save()
{
    FirewalldJob *job = new FirewalldJob(FirewalldJob::SAVEFIREWALLD);

    connect(job, &KJob::result, this, [this, job] {
        if (job->error()) {
            qCDebug(FirewallDClientDebug) << job->name() << job->errorString() << job->error();
            return;
        }
        queryStatus(FirewallClient::DefaultDataBehavior::ReadDefaults, FirewallClient::ProfilesBehavior::DontListenProfiles);
    });
    job->start();
    return job;
};

LogListModel *FirewalldClient::logs()
{
    if (!m_logs) {
        m_logs = new FirewalldLogModel(this);
    }
    return m_logs;
}

QList<Rule *> FirewalldClient::extractRulesFromResponse(const QStringList &reply) const
{
    QList<Rule *> simple_rules;
    if (reply.size() <= 0) {
        return {};
    }
    for (auto r : reply) {
        // ipv4
        simple_rules.push_back(new Rule(Types::POLICY_ALLOW,
                                        true,
                                        Types::LOGGING_OFF,
                                        -1, // TODO retrieve protocol service from firewalld
                                        "0.0.0.0", // passthrough any connection
                                        "0", // TODO retrieve port service from firewalld
                                        "0.0.0.0",
                                        "0",
                                        "",
                                        "",
                                        r, // service name
                                        r, // service name
                                        0, // ignore position
                                        false // ipv family type not relevant to firewalld zone "simple" interface
                                        ));
        // ipv6
        simple_rules.push_back(new Rule(Types::POLICY_ALLOW,
                                        true,
                                        Types::LOGGING_OFF,
                                        -1, // TODO retrieve protocol service from firewalld
                                        "::", // passthrough any connection
                                        "0", // TODO retrieve port service from firewalld
                                        "::",
                                        "0",
                                        "",
                                        "",
                                        r, // service name
                                        r, // service name
                                        0, // ignore position
                                        false // ipv family type not relevant to firewalld zone "simple" interface
                                        ));
    }
    return simple_rules;
}

QList<Rule *> FirewalldClient::extractRulesFromResponse(const QList<firewalld_reply> &reply) const
{
    QList<Rule *> message_rules;
    if (reply.size() <= 0) {
        return {};
    }

    int i = 0;
    for (auto r : reply) {
        const auto action = r.rules.at(r.rules.indexOf("-j") + 1) == "ACCEPT" ? Types::POLICY_ALLOW
            : r.rules.at(r.rules.indexOf("-j") + 1) == "REJECT"               ? Types::POLICY_REJECT
                                                                              : Types::POLICY_DENY;

        const auto sourceAddress = r.rules.indexOf("-s") > 0 ? r.rules.at(r.rules.indexOf("-s") + 1) : "";
        const auto destinationAddress = r.rules.indexOf("-d") >= 0 ? r.rules.at(r.rules.indexOf("-d") + 1) : "";
        const auto interface_in = r.rules.indexOf("-i") >= 0 ? r.rules.at(r.rules.indexOf("-i") + 1) : "";
        const auto interface_out = r.rules.indexOf("-i") >= 0 ? r.rules.at(r.rules.indexOf("-i") + 1) : "";

        if (r.rules.indexOf("-p") < 0) {
            qWarning() << "Error forming rule";
        }

        const QString protocolName = r.rules.at(r.rules.indexOf("-p") + 1);
        const int protocolIdx = FirewallClient::knownProtocols().indexOf(protocolName.toUpper());

        const int sourcePortIdx = r.rules.indexOf(QRegularExpression("^" + QRegularExpression::escape("--sport") + ".+"));
        const auto sourcePort = sourcePortIdx != -1 ? r.rules.at(sourcePortIdx).section("=", -1) : QStringLiteral("");
        const int destPortIdx = r.rules.indexOf(QRegularExpression("^" + QRegularExpression::escape("--dport") + ".+"));
        const auto destPort = destPortIdx != -1 ? r.rules.at(destPortIdx).section("=", -1) : QStringLiteral("");

        message_rules.push_back(new Rule(action,
                                         r.chain == "INPUT",
                                         Types::LOGGING_OFF,
                                         protocolIdx,
                                         sourceAddress,
                                         sourcePort,
                                         destinationAddress,
                                         destPort,
                                         r.chain == "INPUT" ? interface_in : "",
                                         r.chain == "OUTPUT" ? interface_out : "",
                                         "",
                                         "",
                                         i,
                                         r.ipv == "ipv6",
                                         false));
        i += 1;
    }

    return message_rules;
}

void FirewalldClient::setProfile(Profile profile)
{
    auto oldProfile = m_currentProfile;
    m_currentProfile = profile;
    m_rulesModel->setProfile(m_currentProfile);
    qCDebug(FirewallDClientDebug) << "Profile incoming policy: " << m_currentProfile.defaultIncomingPolicy()
                                  << "Old profile policy: " << oldProfile.defaultIncomingPolicy();
    if (m_currentProfile.enabled() != oldProfile.enabled()) {
        getDefaultIncomingPolicyFromDbus();
        Q_EMIT enabledChanged(m_currentProfile.enabled());
    }

    if (enabled()) {
        if (m_currentProfile.defaultIncomingPolicy() != oldProfile.defaultIncomingPolicy()) {
            const QString policy = Types::toString(m_currentProfile.defaultIncomingPolicy());
            Q_EMIT defaultIncomingPolicyChanged(policy);
        }
        if (m_currentProfile.defaultOutgoingPolicy() != oldProfile.defaultOutgoingPolicy()) {
            const QString policy = Types::toString(m_currentProfile.defaultOutgoingPolicy());
            Q_EMIT defaultOutgoingPolicyChanged(policy);
        }
        queryKnownApplications();
    }
}

FirewallClient::Capabilities FirewalldClient::capabilities() const
{
    return FirewallClient::SaveCapability;
};

QStringList FirewalldClient::knownProtocols()
{
    return {"TCP", "UDP"};
}

bool FirewalldClient::isCurrentlyLoaded() const
{
    QProcess process;
    const QString pname = "systemctl";
    const QStringList args = {"status", "firewalld"};

    process.start(pname, args);
    process.waitForFinished();

    // systemctl returns 0 for status if the app is loaded, and 3 otherwise.
    qCDebug(FirewallDClientDebug) << "Firewalld is loaded?" << process.exitCode();

    return process.exitCode() == EXIT_SUCCESS;
}

QString FirewalldClient::version() const
{
    QProcess process;
    QStringList args = {"--version"};

    process.start("firewall-cmd", args);
    process.waitForFinished();

    if (process.exitCode() != EXIT_SUCCESS) {
        return i18n("Error fetching information from the firewall.");
    }

    return process.readAllStandardOutput();
}

QStringList FirewalldClient::knownApplications()
{
    return m_knownApplications;
}

void FirewalldClient::queryKnownApplications()
{
    FirewalldJob *job = new FirewalldJob(FirewalldJob::LISTSERVICES);

    connect(job, &KJob::result, this, [this, job] {
        if (job->error()) {
            qCDebug(FirewallDClientDebug) << job->name() << job->errorString() << job->error();
            return;
        }
        m_knownApplications = job->getServices();
    });
    job->start();
}
void FirewalldClient::getDefaultIncomingPolicyFromDbus()
{
    FirewalldJob *job = new FirewalldJob("getZoneSettings2", {""}, FirewalldJob::SIMPLELIST);
    connect(job, &KJob::result, this, [this, job] {
        if (job->error()) {
            qCDebug(FirewallDClientDebug) << job->name() << job->errorString() << job->error();
            return;
        }
        QString policy = job->getDefaultIncomingPolicy();
        qCDebug(FirewallDClientDebug) << "Incoming Policy (firewalld definition): " << policy;
        if (policy == "default" || policy == "reject") {
            qCDebug(FirewallDClientDebug) << "Setting incoming Policy: rejected";
            m_currentProfile.setDefaultIncomingPolicy("reject");
        } else if (policy == "allow") {
            qCDebug(FirewallDClientDebug) << "Setting incoming Policy: allowed";
            m_currentProfile.setDefaultIncomingPolicy("allow");
        } else {
            qCDebug(FirewallDClientDebug) << "Setting incoming Policy: denied";
            m_currentProfile.setDefaultIncomingPolicy("deny");
        }
    });
    job->exec();
}

#include "firewalldclient.moc"
