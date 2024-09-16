// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DSECRETAGENT_P_H
#define DSECRETAGENT_P_H

#include "dbus/dagentmanagerinterface.h"
#include "dbus/dsecretagentadaptor.h"

DNETWORKMANAGER_BEGIN_NAMESPACE

class DSecretAgent;

class DSecretAgentPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(DSecretAgent)
public:
    explicit DSecretAgentPrivate(const QString &id, DSecretAgent::Capabilities caps, DSecretAgent *parent = nullptr);
    virtual ~DSecretAgentPrivate();

private Q_SLOTS:
    void registerAgent();
    void registerAgent(const DSecretAgent::Capabilities capabilities);
    void dbusInterfacesAdded(const QDBusObjectPath &path, const QVariantMap &interfaces);

private:
    QString m_agentId;
    DSecretAgent::Capabilities m_caps;
    DSecretAgent *q_ptr{nullptr};
    DSecretAgentAdaptor *m_agent{nullptr};
    DAgentManagerInterface *m_manager{nullptr};
};

DNETWORKMANAGER_END_NAMESPACE

#endif
