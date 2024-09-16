// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DAGENTMANAGERINTERFACE_H
#define DAGENTMANAGERINTERFACE_H

#include "dtkbluetooth_global.h"
#include "dbluetoothdbustypes.h"
#include <QObject>
#include <DObject>
#include <QDBusObjectPath>
#include <DDBusInterface>
#include <QDBusPendingReply>

DBLUETOOTH_BEGIN_NAMESPACE

using DTK_CORE_NAMESPACE::DDBusInterface;

class DAgentManagerInterface : public QObject
{
    Q_OBJECT
public:
    explicit DAgentManagerInterface(QObject *parent = nullptr);
    ~DAgentManagerInterface() override = default;

public Q_SLOTS:
    QDBusPendingReply<void> registerAgent(const QDBusObjectPath &agent, const QString &cap);
    QDBusPendingReply<void> requestDefaultAgent(const QDBusObjectPath &agent);
    QDBusPendingReply<void> unregisterAgent(const QDBusObjectPath &agent);

private:
    DDBusInterface *m_inter{nullptr};
};

DBLUETOOTH_END_NAMESPACE

#endif
