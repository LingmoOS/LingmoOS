// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: 2020 Lucas Biaggi <lbjanuario@gmail.com>
/*
 * Firewalld backend for plasma firewall
 */

#ifndef FIREWALLDCLIENT_H
#define FIREWALLDCLIENT_H

#include <QLoggingCategory>
#include <QString>
#include <QTimer>

#include <ifirewallclientbackend.h>
#include <profile.h>

class RuleListModel;
class LogListModel;
struct firewalld_reply;

Q_DECLARE_LOGGING_CATEGORY(FirewallDClientDebug)

class Q_DECL_EXPORT FirewalldClient : public IFirewallClientBackend
{
    Q_OBJECT
public:
    explicit FirewalldClient(QObject *parent, const QVariantList &args);

    void refresh() override;
    RuleListModel *rules() const override;
    Rule *ruleAt(int index) override;
    KJob *addRule(Rule *rule) override;
    KJob *removeRule(int index) override;
    KJob *updateRule(Rule *rule) override;
    KJob *moveRule(int from, int to) override;
    KJob *queryStatus(FirewallClient::DefaultDataBehavior defaultsBehavior, FirewallClient::ProfilesBehavior profilesBehavior) override;
    KJob *setDefaultIncomingPolicy(QString defaultIncomingPolicy) override;
    KJob *setDefaultOutgoingPolicy(QString defaultOutgoingPolicy) override;
    KJob *setEnabled(const bool enabled) override;
    KJob *save() override;
    bool isTcpAndUdp(int protocolIdx) override;

    /* Creates a new Rule and returns it to the Qml side, passing arguments based
     * on the Connection Table. */
    Rule *createRuleFromConnection(const QString &protocol, const QString &localAddress, const QString &foreignAddres, const QString &status) override;

    Rule *createRuleFromLog(const QString &protocol,
                            const QString &sourceAddress,
                            const QString &sourcePort,
                            const QString &destinationAddress,
                            const QString &destinationPort,
                            const QString &inn) override;

    bool enabled() const override;
    QString defaultIncomingPolicy() const override;
    QString defaultOutgoingPolicy() const override;
    QString name() const override;

    FirewallClient::Capabilities capabilities() const override;
    LogListModel *logs() override;
    bool logsAutoRefresh() const override;
    void setLogsAutoRefresh(bool logsAutoRefresh) override;
    static IFirewallClientBackend *createMethod(FirewallClient *parent);
    void refreshProfiles() override;
    QStringList knownProtocols() override;
    bool isCurrentlyLoaded() const override;
    bool supportsRuleUpdate() const override;
    QString version() const override;
    QStringList knownApplications() override;
protected slots:
    void refreshLogs();

protected:
    QList<Rule *> extractRulesFromResponse(const QList<firewalld_reply> &reply) const;
    QList<Rule *> extractRulesFromResponse(const QStringList &reply) const;
    QVariantList buildRule(const Rule *r) const;
    void setProfile(Profile profile);
    void queryKnownApplications();
    void getDefaultIncomingPolicyFromDbus();

private:
    QString m_status;
    QStringList m_rawLogs;
    Profile m_currentProfile;
    RuleListModel *const m_rulesModel;
    LogListModel *m_logs = nullptr;
    QTimer m_logsRefreshTimer;
    bool m_logsAutoRefresh;
    QStringList m_knownApplications;
};

#endif // FIREWALLDCLIENT_H
