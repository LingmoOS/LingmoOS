/*
    SPDX-FileCopyrightText: 2011 Ilia Kats <ilia-kats@gmx.net>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_SECRETAGENT_P_H
#define NETWORKMANAGERQT_SECRETAGENT_P_H

#include <QDBusServiceWatcher>

#include "agentmanagerinterface.h"
#include "secretagentadaptor.h"

namespace NetworkManager
{
class SecretAgent;
class SecretAgentPrivate
{
    Q_DECLARE_PUBLIC(SecretAgent)
public:
    explicit SecretAgentPrivate(const QString &, SecretAgent *parent = nullptr);
    explicit SecretAgentPrivate(const QString &, const NetworkManager::SecretAgent::Capabilities, SecretAgent *parent = nullptr);
    virtual ~SecretAgentPrivate();
private Q_SLOTS:
    void dbusInterfacesAdded(const QDBusObjectPath &path, const QVariantMap &interfaces);
    void daemonRegistered();
    void registerAgent();
    void registerAgent(const NetworkManager::SecretAgent::Capabilities capabilities);

private:
    SecretAgent *q_ptr;
    SecretAgentAdaptor agent;
    OrgFreedesktopNetworkManagerAgentManagerInterface agentManager;
    QDBusServiceWatcher watcher;
    QString agentId;
    NetworkManager::SecretAgent::Capabilities capabilities;
};
}

#endif // NETWORKMANAGERQT_SECRETAGENT_P_H