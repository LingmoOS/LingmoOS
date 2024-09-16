// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SECRETAGENTSERVICE_H
#define SECRETAGENTSERVICE_H

#include <QDBusObjectPath>
#include "dnetworkmanagertypes.h"

DNETWORKMANAGER_USE_NAMESPACE

class FakeSecretAgentService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.FakeNetworkManager.SecretAgent")
public:
    explicit FakeSecretAgentService(QObject *parent = nullptr);
    ~FakeSecretAgentService() override;

    bool m_getSecretTrigger{false};
    bool m_cancelGetSecretTrigger{false};
    bool m_saveSecretTrigger{false};
    bool m_deleteSecretTrigger{false};

public Q_SLOTS:

    Q_SCRIPTABLE SettingDesc GetSecrets(SettingDesc, QDBusObjectPath, QString, QStringList, quint32)
    {
        m_getSecretTrigger = true;
        return {};
    }

    Q_SCRIPTABLE void CancelGetSecrets(QDBusObjectPath, QString) { m_cancelGetSecretTrigger = true; }

    Q_SCRIPTABLE void SaveSecrets(SettingDesc, QDBusObjectPath) { m_saveSecretTrigger = true; }

    Q_SCRIPTABLE void DeleteSecrets(SettingDesc, QDBusObjectPath) { m_deleteSecretTrigger = true; };

private:
    void registerService();
    void unregisterService();
    const QString Service{"com.deepin.FakeNetworkManager"};
    const QString Path{"/com/deepin/FakeNetworkManager/SecretAgent"};
};

#endif
