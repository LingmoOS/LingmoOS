// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DSECRETAGENT_H
#define DSECRETAGENT_H

#include "dnetworkmanagertypes.h"
#include <DExpected>
#include <QFlags>
#include <QDBusMessage>
#include <QDBusContext>
#include <QDBusObjectPath>

DNETWORKMANAGER_BEGIN_NAMESPACE

class DSecretAgentPrivate;

using DCORE_NAMESPACE::DExpected;

class DSecretAgent : public QObject, protected QDBusContext
{
    Q_OBJECT
public:
    Q_DECLARE_FLAGS(GetSecretFlags, NMSecretAgentGetSecretsFlags);
    Q_DECLARE_FLAGS(Capabilities, NMSecretAgentCapabilities)

    explicit DSecretAgent(const QString &id, QObject *parent = nullptr);
    explicit DSecretAgent(const QString &id, Capabilities caps, QObject *parent = nullptr);
    ~DSecretAgent() override;

public Q_SLOTS:
    virtual DExpected<SettingDesc> secrets(const SettingDesc &conn,
                                           const quint64 connId,
                                           const QString &settingName,
                                           const QList<QString> &hints,
                                           const GetSecretFlags &flags) = 0;
    virtual DExpected<void> cancelSecrets(const quint64 connId, const QString &settingName) = 0;
    virtual DExpected<void> saveSecret(const SettingDesc &connSettigns, const quint64 connId) = 0;
    virtual DExpected<void> deleteSecret(const SettingDesc &connSettigns, const quint64 connId) = 0;

private:
    Q_DECLARE_PRIVATE(DSecretAgent)
    Q_PRIVATE_SLOT(d_func(), void registerAgent())
    Q_PRIVATE_SLOT(d_func(), void registerAgent(const DSecretAgent::Capabilities capabilities))
    Q_PRIVATE_SLOT(d_func(), void dbusInterfacesAdded(const QDBusObjectPath &path, const QVariantMap &interfaces))
    QScopedPointer<DSecretAgentPrivate> d_ptr;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(DSecretAgent::GetSecretFlags)
Q_DECLARE_OPERATORS_FOR_FLAGS(DSecretAgent::Capabilities)

DNETWORKMANAGER_END_NAMESPACE

#endif
