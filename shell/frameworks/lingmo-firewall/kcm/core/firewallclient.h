// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: 2011 Craig Drummond <craig.p.drummond@gmail.com>
// SPDX-FileCopyrightText: 2018 Alexis Lopes Zubeta <contact@azubieta.net>
// SPDX-FileCopyrightText: 2020 Tomaz Canabrava <tcanabrava@kde.org>
/*
 * UFW KControl Module
 */

#pragma once

#include <kcm_firewall_core_export.h>

#include <QFlags>
#include <QObject>
#include <QTimer>

#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(FirewallClientDebug)

class KJob;
class RuleListModel;
class LogListModel;
class IFirewallClientBackend;
class Rule;

/* This class is the entry point of the Firewall KCM
 * It uses internal FirewallImplementations defined in
 * the backend/ folder.
 *
 * To setup a firewall, this will look first for "higher abstractions"
 * like firewalld and ufw, then bsd specifics, etc.
 */

class KCM_FIREWALL_CORE_EXPORT FirewallClient : public QObject
{
    Q_OBJECT
    /**
     * Whether the firewall is enabled
     */
    Q_PROPERTY(bool enabled READ enabled NOTIFY enabledChanged)
    Q_PROPERTY(QString defaultIncomingPolicy READ defaultIncomingPolicy NOTIFY defaultIncomingPolicyChanged)
    Q_PROPERTY(QString defaultOutgoingPolicy READ defaultOutgoingPolicy NOTIFY defaultOutgoingPolicyChanged)
    Q_PROPERTY(RuleListModel *rulesModel READ rulesModel CONSTANT)
    Q_PROPERTY(LogListModel *logsModel READ logsModel CONSTANT)
    Q_PROPERTY(bool logsAutoRefresh READ logsAutoRefresh WRITE setLogsAutoRefresh NOTIFY logsAutoRefreshChanged)
    Q_PROPERTY(bool hasExecutable READ hasExecutable NOTIFY hasExecutableChanged)
    Q_PROPERTY(Capabilities capabilities READ capabilities NOTIFY capabilitiesChanged)
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(bool supportsRuleUpdate READ supportsRuleUpdate CONSTANT)

public:
    enum DefaultDataBehavior { DontReadDefaults, ReadDefaults };
    enum ProfilesBehavior { DontListenProfiles, ListenProfiles };
    explicit FirewallClient(QObject *parent = nullptr);
    ~FirewallClient();

    void setBackend(const QStringList &backendList);

    Q_INVOKABLE static QStringList knownProtocols();
    Q_INVOKABLE static QStringList knownInterfaces();
    Q_INVOKABLE QStringList knownApplications();
    Q_INVOKABLE static bool isTcpAndUdp(int protocolIdx);
    static int indexOfProtocol(const QString &protocol);

    Q_INVOKABLE void refresh();

    RuleListModel *rulesModel() const;
    LogListModel *logsModel() const;

    Q_INVOKABLE Rule *ruleAt(int index); // TODO move into the model?
    Q_INVOKABLE KJob *addRule(Rule *rule);
    Q_INVOKABLE KJob *removeRule(int index);
    Q_INVOKABLE KJob *updateRule(Rule *rule);
    Q_INVOKABLE KJob *moveRule(int from, int to);

    Q_INVOKABLE KJob *setEnabled(bool enabled);
    Q_INVOKABLE KJob *setDefaultIncomingPolicy(const QString &defaultIncomingPolicy);
    Q_INVOKABLE KJob *setDefaultOutgoingPolicy(const QString &defaultOutgoingPolicy);
    Q_INVOKABLE KJob *save();
    /* Creates a new Rule and returns it to the Qml side, passing arguments based on the Connection Table. */
    Q_INVOKABLE Rule *createRuleFromConnection(const QString &protocol, const QString &localAddress, const QString &foreignAddres, const QString &status);

    Q_INVOKABLE Rule *createRuleFromLog(const QString &protocol,
                                        const QString &sourceAddress,
                                        const QString &sourcePort,
                                        const QString &destinationAddress,
                                        const QString &destinationPort,
                                        const QString &inn);

    Q_INVOKABLE QString version() const;

    bool enabled() const;
    bool hasExecutable() const;
    QString name() const;
    QString defaultIncomingPolicy() const;
    QString defaultOutgoingPolicy() const;
    QString backend() const;
    bool logsAutoRefresh() const;
    bool supportsRuleUpdate() const;
    enum Capability {
        None = 0x0,
        SaveCapability = 0x1,
    };

    Q_ENUM(Capability)
    Q_DECLARE_FLAGS(Capabilities, Capability)
    Q_FLAG(Capabilities);

    Capabilities capabilities() const;

Q_SIGNALS:
    void enabledChanged(const bool enabled);
    void defaultIncomingPolicyChanged(const QString &defaultIncomingPolicy);
    void defaultOutgoingPolicyChanged(const QString &defaultOutgoingPolicy);
    void logsAutoRefreshChanged(bool logsAutoRefresh);
    void backendChanged(const QString &backend);
    void hasExecutableChanged(bool changed);
    void capabilitiesChanged(const FirewallClient::Capabilities &capabilities);
    /**
     * Emitted when an error message should be displayed.
     *
     * This is typically shown as an inline message, e.g. "Failed to create action: Not authorized."
     */
    void showErrorMessage(const QString &message);

private:
    void setLogsAutoRefresh(bool logsAutoRefresh);
    void queryStatus(DefaultDataBehavior defaultDataBehavior = ReadDefaults, ProfilesBehavior ProfilesBehavior = ListenProfiles);

    static IFirewallClientBackend *m_currentBackend;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(FirewallClient::Capabilities)
