// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dsecretagentadaptor.h"
#include "dnmutils.h"

DNETWORKMANAGER_BEGIN_NAMESPACE

DSecretAgentAdaptor::DSecretAgentAdaptor(DSecretAgent *parent)
    : QDBusAbstractAdaptor(parent)
{
    setAutoRelaySignals(true);
}

DSecretAgentAdaptor::~DSecretAgentAdaptor() = default;

void DSecretAgentAdaptor::CancelGetSecrets(const QDBusObjectPath &connection_path, const QString &setting_name)
{
    parent()->cancelSecrets(getIdFromObjectPath(connection_path), setting_name);
}

void DSecretAgentAdaptor::DeleteSecrets(SettingDesc connection, const QDBusObjectPath &connection_path)
{
    parent()->deleteSecret(connection, getIdFromObjectPath(connection_path));
}

SettingDesc DSecretAgentAdaptor::GetSecrets(SettingDesc connection,
                                            const QDBusObjectPath &connection_path,
                                            const QString &setting_name,
                                            const QStringList &hints,
                                            uint flags)
{
    auto ret = parent()->secrets(
        connection, getIdFromObjectPath(connection_path), setting_name, hints, DSecretAgent::GetSecretFlags(flags));
    if (!ret) {
        return {};
    }
    return ret.value();
}

void DSecretAgentAdaptor::SaveSecrets(SettingDesc connection, const QDBusObjectPath &connection_path)
{
    parent()->saveSecret(connection, getIdFromObjectPath(connection_path));
}

DNETWORKMANAGER_END_NAMESPACE
