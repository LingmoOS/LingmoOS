// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dsecretagent.h"
#include "dsecretagent_p.h"

#undef signals
#include <NetworkManager.h>
#include <libnm/NetworkManager.h>

#define signals Q_SIGNALS

#include <QDBusConnection>

DNETWORKMANAGER_BEGIN_NAMESPACE

DSecretAgentPrivate::DSecretAgentPrivate(const QString &id, DSecretAgent::Capabilities caps, DSecretAgent *parent)
    : m_agentId(id)
    , m_caps(caps)
    , q_ptr(parent)
    , m_agent(new DSecretAgentAdaptor(parent))
    , m_manager(new DAgentManagerInterface(this))
{
    Q_Q(DSecretAgent);
    qRegisterMetaType<SettingDesc>("SettingDesc");
    qDBusRegisterMetaType<SettingDesc>();
    auto managerConnection = m_manager->getConnection();
    managerConnection.connect("org.freedesktop.NetworkManager",
                              "/org/freedesktop",
                              "org.freedesktop.DBus.ObjectManager",
                              QLatin1String("InterfacesAdded"),
                              q,
                              SLOT(dbusInterfacesAdded(QDBusObjectPath, QVariantMap)));
    managerConnection.registerObject(QLatin1String(NM_DBUS_PATH_SECRET_AGENT), m_agent, QDBusConnection::ExportAllSlots);
    registerAgent(caps);
}

DSecretAgentPrivate::~DSecretAgentPrivate()
{
    m_manager->unregisterAgent();
}

void DSecretAgentPrivate::dbusInterfacesAdded(const QDBusObjectPath &path, const QVariantMap &interfaces)
{
    Q_UNUSED(path);
    if (!interfaces.contains(QString::fromLatin1("org.freedesktop.NetworkManager.AgentManager"))) {
        return;
    }

    registerAgent(m_caps);
}

void DSecretAgentPrivate::registerAgent()
{
    m_manager->registerAgent(m_agentId);
}

void DSecretAgentPrivate::registerAgent(const DSecretAgent::Capabilities capabilities)
{
    m_manager->registerWithCapabilities(m_agentId, capabilities);
}

DSecretAgent::DSecretAgent(const QString &id, QObject *parent)
    : QObject(parent)
    , QDBusContext()
    , d_ptr(new DSecretAgentPrivate(id, NMSecretAgentCapabilities::None, this))
{
}

DSecretAgent::DSecretAgent(const QString &id, Capabilities caps, QObject *parent)
    : QObject(parent)
    , QDBusContext()
    , d_ptr(new DSecretAgentPrivate(id, caps, this))
{
}

DSecretAgent::~DSecretAgent() = default;

#include "moc_dsecretagent.cpp"

DNETWORKMANAGER_END_NAMESPACE
