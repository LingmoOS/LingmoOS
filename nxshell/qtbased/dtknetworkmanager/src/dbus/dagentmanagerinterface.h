// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DAGENTMANAGERINTERFACE_H
#define DAGENTMANAGERINTERFACE_H

#include "dsecretagent.h"
#include "dnetworkmanager_global.h"
#include <QObject>
#include <QString>
#include <DDBusInterface>
#include <QDBusPendingReply>

DNETWORKMANAGER_BEGIN_NAMESPACE

using DTK_CORE_NAMESPACE::DDBusInterface;

class DAgentManagerInterface : public QObject
{
    Q_OBJECT
public:
    explicit DAgentManagerInterface(QObject *parent = nullptr);
    ~DAgentManagerInterface() override = default;

public:
    auto getConnection() { return m_inter->connection(); }

public Q_SLOTS:

    QDBusPendingReply<void> registerAgent(const QString &identifier) const;
    QDBusPendingReply<void> registerWithCapabilities(const QString &identifier, const DSecretAgent::Capabilities caps) const;
    QDBusPendingReply<void> unregisterAgent() const;

private:
    DDBusInterface *m_inter{nullptr};
};

DNETWORKMANAGER_END_NAMESPACE

#endif
