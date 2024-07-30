// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: 2011 Craig Drummond <craig.p.drummond@gmail.com>
// SPDX-FileCopyrightText: 2018 Alexis Lopes Zubeta <contact@azubieta.net>
// SPDX-FileCopyrightText: 2020 Tomaz Canabrava <tcanabrava@kde.org>
/*
 * UFW KControl Module
 */

#pragma once

#include <QString>
#include <QTimer>

#include <KAuth/Action>

#include <QXmlStreamWriter>
#include <ifirewallclientbackend.h>
#include <profile.h>

class RuleListModel;
class LogListModel;

class Q_DECL_EXPORT UfwClient : public IFirewallClientBackend
{
    Q_OBJECT
public:
    explicit UfwClient(QObject *parent, const QVariantList &args);

    void refresh() override;
    RuleListModel *rules() const override;
    Rule *ruleAt(int index) override;
    KJob *addRule(Rule *r) override;
    KJob *removeRule(int index) override;
    KJob *updateRule(Rule *r) override;
    KJob *moveRule(int from, int to) override;
    bool isTcpAndUdp(int protocolIdx) override;

    KJob *queryStatus(FirewallClient::DefaultDataBehavior defaultsBehavior, FirewallClient::ProfilesBehavior profilesBehavior) override;
    KJob *setDefaultIncomingPolicy(QString defaultIncomingPolicy) override;
    KJob *setDefaultOutgoingPolicy(QString defaultOutgoingPolicy) override;

    KJob *setEnabled(bool enabled) override;

    /* Creates a new Rule and returns it to the Qml side, passing arguments based on the Connection Table. */
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

    LogListModel *logs() override;
    bool logsAutoRefresh() const override;
    void setLogsAutoRefresh(bool logsAutoRefresh) override;
    static IFirewallClientBackend *createMethod(FirewallClient *parent);
    void refreshProfiles() override;
    bool isCurrentlyLoaded() const override;
    bool supportsRuleUpdate() const override;

    QString version() const override;
    QStringList knownApplications() override;

protected slots:
    void refreshLogs();
    QStringList knownProtocols() override;

protected:
    void setProfile(Profile profile);
    void queryKnownApplications();
    KAuth::Action buildQueryAction(const QVariantMap &arguments);
    KAuth::Action buildModifyAction(const QVariantMap &arguments);

private:
    void enableService(bool value);
    QString toXml(Rule *r) const;
    QStringList m_rawLogs;
    Profile m_currentProfile;
    RuleListModel *const m_rulesModel;
    LogListModel *m_logs = nullptr;
    QTimer m_logsRefreshTimer;
    bool m_logsAutoRefresh;
    KAuth::Action m_queryAction;
    bool m_busy = false;
    QStringList m_knownApplications;
};
