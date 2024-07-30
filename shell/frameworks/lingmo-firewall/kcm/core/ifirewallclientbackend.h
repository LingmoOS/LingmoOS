// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: 2011 Craig Drummond <craig.p.drummond@gmail.com>
// SPDX-FileCopyrightText: 2018 Alexis Lopes Zubeta <contact@azubieta.net>
// SPDX-FileCopyrightText: 2020 Tomaz Canabrava <tcanabrava@kde.org>

/*
 * UFW KControl Module
 */

#ifndef IFIREWALLCLIENTBACKEND_H
#define IFIREWALLCLIENTBACKEND_H

#include "appprofiles.h"
#include "firewallclient.h"

#include <QList>
#include <QString>
#include <kcm_firewall_core_export.h>

class KJob;
class LogListModel;
class Rule;
class RuleListModel;
class FirewallClient;

class KCM_FIREWALL_CORE_EXPORT IFirewallClientBackend : public QObject
{
    Q_OBJECT
public:
    IFirewallClientBackend(QObject *parent, const QVariantList &args);
    ~IFirewallClientBackend() = default;

    virtual QString name() const = 0;
    virtual void refresh() = 0;
    virtual RuleListModel *rules() const = 0;
    virtual bool isTcpAndUdp(int protocolIdx) = 0;
    virtual Rule *ruleAt(int index) = 0;

    Q_INVOKABLE virtual KJob *addRule(Rule *rule) = 0;
    Q_INVOKABLE virtual KJob *removeRule(int index) = 0;
    Q_INVOKABLE virtual KJob *updateRule(Rule *rule) = 0;
    Q_INVOKABLE virtual KJob *moveRule(int from, int to) = 0;

    Q_INVOKABLE virtual KJob *setEnabled(bool enabled) = 0;
    Q_INVOKABLE virtual KJob *queryStatus(FirewallClient::DefaultDataBehavior defaultsBehavior, FirewallClient::ProfilesBehavior profilesBehavior) = 0;
    Q_INVOKABLE virtual KJob *setDefaultIncomingPolicy(QString defaultIncomingPolicy) = 0;
    Q_INVOKABLE virtual KJob *setDefaultOutgoingPolicy(QString defaultOutgoingPolicy) = 0;
    Q_INVOKABLE virtual KJob *save();

    Q_INVOKABLE virtual bool supportsRuleUpdate() const = 0;

    /* returns the --version of the software
     * the base runs `firewallbackend --help`
     * firewalld does not have --help. :|
     */
    virtual QString version() const = 0;

    virtual void setLogsAutoRefresh(bool logsAutoRefresh) = 0;

    /* Creates a new Rule and returns it to the Qml side, passing arguments based on the Connection Table. */
    virtual Rule *createRuleFromConnection(const QString &protocol, const QString &localAddress, const QString &foreignAddres, const QString &status) = 0;

    virtual Rule *createRuleFromLog(const QString &protocol,
                                    const QString &sourceAddress,
                                    const QString &sourcePort,
                                    const QString &destinationAddress,
                                    const QString &destinationPort,
                                    const QString &inn) = 0;

    virtual bool enabled() const = 0;
    virtual QString defaultIncomingPolicy() const = 0;
    virtual QString defaultOutgoingPolicy() const = 0;
    virtual LogListModel *logs() = 0;

    /* TODO: Move it away from here. This asks the
     * Logs model to refresh after a few seconds
     */
    virtual bool logsAutoRefresh() const = 0;

    /* Returns true if the firewall represented
     * by this backend is running right now */
    virtual bool isCurrentlyLoaded() const = 0;

    virtual void refreshProfiles() = 0;
    virtual FirewallClient::Capabilities capabilities() const;
    virtual QStringList knownProtocols() = 0;
    virtual QStringList knownApplications() = 0;

    void setProfiles(const QList<Entry> &profiles);
    QList<Entry> profiles();
    Entry profileByName(const QString &profileName);

    void queryExecutable(const QString &executableName);
    bool hasExecutable() const;
    QString executablePath() const;
Q_SIGNALS:
    void enabledChanged(bool enabled);
    void defaultIncomingPolicyChanged(const QString &defaultIncomingPolicy);
    void defaultOutgoingPolicyChanged(const QString &defaultOutgoingPolicy);
    void logsAutoRefreshChanged(bool logsAutoRefresh);
    // Is this even used?
    void hasExecutableChanged(bool changed);

    // TODO is this needed?
    void showErrorMessage(const QString &message);

private:
    QList<Entry> m_profiles;
    QString m_executablePath;
};

#endif
