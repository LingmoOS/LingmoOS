// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef AGENTMANAGERSERVICE_H
#define AGENTMANAGERSERVICE_H

#include <QDBusObjectPath>

class FakeAgentManagerService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.FakeNetworkManager.AgentManager")
public:
    explicit FakeAgentManagerService(QObject *parent = nullptr);
    ~FakeAgentManagerService() override;

    bool m_registerTrigger{false};
    bool m_registerWithCapabilitiesTrigger{false};
    bool m_unregisterTrigger{false};

public Q_SLOTS:
    Q_SCRIPTABLE void Register(QByteArray) { m_registerTrigger = true; }

    Q_SCRIPTABLE void RegisterWithCapabilities(QByteArray, quint32) { m_registerWithCapabilitiesTrigger = true; }

    Q_SCRIPTABLE void Unregister() { m_unregisterTrigger = true; }

private:
    void registerService();
    void unregisterService();
    const QString Service{"com.deepin.FakeNetworkManager"};
    const QString Path{"/com/deepin/FakeNetworkManager/AgentManager"};
};

#endif
