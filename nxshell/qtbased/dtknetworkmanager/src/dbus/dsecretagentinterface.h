// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DSECRETAGENTINTERFACE_H
#define DSECRETAGENTINTERFACE_H

#include "dnetworkmanagertypes.h"
#include "dnetworkmanager_global.h"
#include <QObject>
#include <QString>
#include <DDBusInterface>
#include <QDBusPendingReply>

DNETWORKMANAGER_BEGIN_NAMESPACE

using DTK_CORE_NAMESPACE::DDBusInterface;

class DSecretAgentInterface : public QObject
{
    Q_OBJECT
public:
    explicit DSecretAgentInterface(QObject *parent = nullptr);
    ~DSecretAgentInterface() override = default;

public Q_SLOTS:
    QDBusPendingReply<SettingDesc> getSecrets(const SettingDesc &connection,
                                              const QByteArray &connectionPath,
                                              const QByteArray &settingName,
                                              const QList<QByteArray> &hints,
                                              const quint32 flag) const;
    QDBusPendingReply<void> cancelGetSecrets(const QByteArray &connectionPath, const QByteArray &settingName) const;
    QDBusPendingReply<void> saveSecrets(const SettingDesc &connection, const QByteArray &connectionPath) const;
    QDBusPendingReply<void> deleteSecrets(const SettingDesc &connection, const QByteArray &connectionPath) const;

private:
    DDBusInterface *m_inter{nullptr};
};

DNETWORKMANAGER_END_NAMESPACE

#endif
