/*
    SPDX-FileCopyrightText: 2011 Ilia Kats <ilia-kats@gmx.net>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "secretagent.h"
#include "manager_p.h"
#include "nmdebug.h"
#include "secretagent_p.h"

#undef signals
#include <NetworkManager.h>
#include <libnm/NetworkManager.h>

#define signals Q_SIGNALS

#include <QDBusConnection>
#include <QDBusMetaType>

#include "agentmanagerinterface.h"
#include "secretagentadaptor.h"

NetworkManager::SecretAgentPrivate::SecretAgentPrivate(const QString &id,
                                                       const NetworkManager::SecretAgent::Capabilities capabilities,
                                                       NetworkManager::SecretAgent *parent)
    : q_ptr(parent)
    , agent(parent)
#ifdef NMQT_STATIC
    , agentManager(NetworkManagerPrivate::DBUS_SERVICE, QLatin1String(NM_DBUS_PATH_AGENT_MANAGER), QDBusConnection::sessionBus(), parent)
    , watcher(NetworkManagerPrivate::DBUS_SERVICE, QDBusConnection::sessionBus(), QDBusServiceWatcher::WatchForOwnerChange, parent)
#else
    , agentManager(NetworkManagerPrivate::DBUS_SERVICE, QLatin1String(NM_DBUS_PATH_AGENT_MANAGER), QDBusConnection::systemBus(), parent)
    , watcher(NetworkManagerPrivate::DBUS_SERVICE, QDBusConnection::systemBus(), QDBusServiceWatcher::WatchForOwnerChange, parent)
#endif
    , agentId(id)
    , capabilities(capabilities)
{
    Q_Q(SecretAgent);

    qRegisterMetaType<NMVariantMapMap>("NMVariantMapMap");
    qDBusRegisterMetaType<NMVariantMapMap>();

    // Similarly to NetworkManagerPrivate(), we register two listeners here:
    // one for ServiceRegistered and one for InterfacesAdded.
    agentManager.connection().connect(NetworkManagerPrivate::DBUS_SERVICE,
                                      "/org/freedesktop",
                                      NetworkManagerPrivate::FDO_DBUS_OBJECT_MANAGER,
                                      QLatin1String("InterfacesAdded"),
                                      q,
                                      SLOT(dbusInterfacesAdded(QDBusObjectPath, QVariantMap)));

    QObject::connect(&watcher, &QDBusServiceWatcher::serviceRegistered, parent, [this] { this->daemonRegistered(); });

    agentManager.connection().registerObject(QLatin1String(NM_DBUS_PATH_SECRET_AGENT), &agent, QDBusConnection::ExportAllSlots);

    registerAgent(capabilities);
}

NetworkManager::SecretAgentPrivate::~SecretAgentPrivate()
{
    agentManager.Unregister();
}

void NetworkManager::SecretAgentPrivate::dbusInterfacesAdded(const QDBusObjectPath &path, const QVariantMap &interfaces)
{
    Q_UNUSED(path);
    if (!interfaces.contains(QString::fromLatin1(agentManager.staticInterfaceName()))) {
        return;
    }

    registerAgent(capabilities);
}

void NetworkManager::SecretAgentPrivate::daemonRegistered() 
{
    registerAgent(capabilities);
}

void NetworkManager::SecretAgentPrivate::registerAgent()
{
    registerAgent(NetworkManager::SecretAgent::Capability::NoCapability);
}

void NetworkManager::SecretAgentPrivate::registerAgent(NetworkManager::SecretAgent::Capabilities capabilities)
{
    agentManager.RegisterWithCapabilities(agentId, capabilities);
}

NetworkManager::SecretAgent::SecretAgent(const QString &id, QObject *parent)
    : QObject(parent)
    , QDBusContext()
    , d_ptr(new NetworkManager::SecretAgentPrivate(id, NetworkManager::SecretAgent::Capability::NoCapability, this))
{
}

NetworkManager::SecretAgent::SecretAgent(const QString &id, NetworkManager::SecretAgent::Capabilities capabilities, QObject *parent)
    : QObject(parent)
    , QDBusContext()
    , d_ptr(new NetworkManager::SecretAgentPrivate(id, capabilities, this))
{
}

NetworkManager::SecretAgent::~SecretAgent()
{
    Q_D(SecretAgent);
    delete d;
}

void NetworkManager::SecretAgent::sendError(NetworkManager::SecretAgent::Error error, const QString &explanation, const QDBusMessage &callMessage) const
{
    Q_D(const SecretAgent);

    QString errorString;
    switch (error) {
    case NotAuthorized:
        errorString = QStringLiteral(NM_DBUS_INTERFACE_SECRET_AGENT) % QStringLiteral(".NotAuthorized");
        break;
    case InvalidConnection:
        errorString = QStringLiteral(NM_DBUS_INTERFACE_SECRET_AGENT) % QStringLiteral(".InvalidConnection");
        break;
    case UserCanceled:
        errorString = QStringLiteral(NM_DBUS_INTERFACE_SECRET_AGENT) % QStringLiteral(".UserCanceled");
        break;
    case AgentCanceled:
        errorString = QStringLiteral(NM_DBUS_INTERFACE_SECRET_AGENT) % QStringLiteral(".AgentCanceled");
        break;
    case InternalError:
        errorString = QStringLiteral(NM_DBUS_INTERFACE_SECRET_AGENT) % QStringLiteral(".InternalError");
        break;
    case NoSecrets:
        errorString = QStringLiteral(NM_DBUS_INTERFACE_SECRET_AGENT) % QStringLiteral(".NoSecrets");
        break;
    default:
        errorString = QStringLiteral(NM_DBUS_INTERFACE_SECRET_AGENT) % QStringLiteral(".Unknown");
        break;
    }

    QDBusMessage reply;
    if (callMessage.type() == QDBusMessage::InvalidMessage) {
        reply = message().createErrorReply(errorString, explanation);
    } else {
        reply = callMessage.createErrorReply(errorString, explanation);
    }

    if (!d->agentManager.connection().send(reply)) {
        qCDebug(NMQT) << "Failed to put error message on DBus queue" << errorString << explanation;
    }
}

#include "moc_secretagent.cpp"
