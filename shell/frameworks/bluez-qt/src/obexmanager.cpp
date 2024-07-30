/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * SPDX-FileCopyrightText: 2014 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "obexmanager.h"
#include "debug.h"
#include "initobexmanagerjob.h"
#include "obexagent.h"
#include "obexagentadaptor.h"
#include "obexmanager_p.h"
#include "obexsession.h"
#include "pendingcall.h"
#include "utils.h"

#include <QDBusServiceWatcher>

namespace BluezQt
{
ObexManager::ObexManager(QObject *parent)
    : QObject(parent)
    , d(new ObexManagerPrivate(this))
{
    Instance::setObexManager(this);
}

ObexManager::~ObexManager() = default;

InitObexManagerJob *ObexManager::init()
{
    return new InitObexManagerJob(this);
}

bool ObexManager::isInitialized() const
{
    return d->m_initialized;
}

bool ObexManager::isOperational() const
{
    return d->m_initialized && d->m_obexRunning && d->m_loaded;
}

QList<ObexSessionPtr> ObexManager::sessions() const
{
    return d->m_sessions.values();
}

ObexSessionPtr ObexManager::sessionForPath(const QDBusObjectPath &path) const
{
    for (ObexSessionPtr session : std::as_const(d->m_sessions)) {
        if (path.path().startsWith(session->objectPath().path())) {
            return session;
        }
    }

    return ObexSessionPtr();
}

PendingCall *ObexManager::startService()
{
    QDBusMessage msg = QDBusMessage::createMethodCall(Strings::orgFreedesktopDBus(),
                                                      QStringLiteral("/org/freedesktop/DBus"),
                                                      Strings::orgFreedesktopDBus(),
                                                      QStringLiteral("StartServiceByName"));
    msg << Strings::orgBluezObex();
    msg << quint32(0);

    return new PendingCall(DBusConnection::orgBluezObex().asyncCall(msg), PendingCall::ReturnUint32);
}

PendingCall *ObexManager::registerAgent(ObexAgent *agent)
{
    Q_ASSERT(agent);

    if (!d->m_obexAgentManager) {
        return new PendingCall(PendingCall::InternalError, QStringLiteral("ObexManager not operational!"));
    }

    new ObexAgentAdaptor(agent, this);

    if (!DBusConnection::orgBluezObex().registerObject(agent->objectPath().path(), agent)) {
        qCDebug(BLUEZQT) << "Cannot register object" << agent->objectPath().path();
    }

    return new PendingCall(d->m_obexAgentManager->RegisterAgent(agent->objectPath()), PendingCall::ReturnVoid, this);
}

PendingCall *ObexManager::unregisterAgent(ObexAgent *agent)
{
    Q_ASSERT(agent);

    if (!d->m_obexAgentManager) {
        return new PendingCall(PendingCall::InternalError, QStringLiteral("ObexManager not operational!"));
    }

    DBusConnection::orgBluezObex().unregisterObject(agent->objectPath().path());

    return new PendingCall(d->m_obexAgentManager->UnregisterAgent(agent->objectPath()), PendingCall::ReturnVoid, this);
}

PendingCall *ObexManager::createSession(const QString &destination, const QVariantMap &args)
{
    if (!d->m_obexClient) {
        return new PendingCall(PendingCall::InternalError, QStringLiteral("ObexManager not operational!"));
    }

    return new PendingCall(d->m_obexClient->CreateSession(destination, args), PendingCall::ReturnObjectPath, this);
}

PendingCall *ObexManager::removeSession(const QDBusObjectPath &session)
{
    if (!d->m_obexClient) {
        return new PendingCall(PendingCall::InternalError, QStringLiteral("ObexManager not operational!"));
    }

    return new PendingCall(d->m_obexClient->RemoveSession(session), PendingCall::ReturnVoid, this);
}

} // namespace BluezQt

#include "moc_obexmanager.cpp"
