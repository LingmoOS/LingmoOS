// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DOBEXAGENTMANAGERINTERFACE_H
#define DOBEXAGENTMANAGERINTERFACE_H

#include "dtkbluetooth_global.h"
#include <QObject>
#include <QDBusObjectPath>
#include <DDBusInterface>
#include <QDBusPendingReply>

DBLUETOOTH_BEGIN_NAMESPACE

using DTK_CORE_NAMESPACE::DDBusInterface;

class DObexAgentManagerInterface : public QObject
{
    Q_OBJECT
public:
    explicit DObexAgentManagerInterface(QObject *parent = nullptr);
    ~DObexAgentManagerInterface() override = default;

public Q_SLOTS:

    QDBusPendingReply<void> registerAgent(const QDBusObjectPath &agent) const;
    QDBusPendingReply<void> unregisterAgent(const QDBusObjectPath &agent) const;

private:
    DDBusInterface *m_inter{nullptr};
};

DBLUETOOTH_END_NAMESPACE

#endif
