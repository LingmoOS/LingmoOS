/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * SPDX-FileCopyrightText: 2014-2015 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "obexmanager_p.h"
#include "debug.h"
#include "obexmanager.h"
#include "obexsession.h"
#include "obexsession_p.h"
#include "utils.h"

#include <QDBusServiceWatcher>

namespace BluezQt
{
typedef org::freedesktop::DBus::ObjectManager DBusObjectManager;

ObexManagerPrivate::ObexManagerPrivate(ObexManager *qq)
    : QObject(qq)
    , q(qq)
    , m_obexClient(nullptr)
    , m_obexAgentManager(nullptr)
    , m_dbusObjectManager(nullptr)
    , m_initialized(false)
    , m_obexRunning(false)
    , m_loaded(false)
{
    qDBusRegisterMetaType<DBusManagerStruct>();
    qDBusRegisterMetaType<QVariantMapMap>();

    m_timer.setSingleShot(true);
    connect(&m_timer, &QTimer::timeout, this, &ObexManagerPrivate::load);
}

void ObexManagerPrivate::init()
{
    // Keep an eye on org.bluez.obex service
    QDBusServiceWatcher *serviceWatcher = new QDBusServiceWatcher(Strings::orgBluezObex(),
                                                                  DBusConnection::orgBluezObex(),
                                                                  QDBusServiceWatcher::WatchForRegistration | QDBusServiceWatcher::WatchForUnregistration,
                                                                  this);

    connect(serviceWatcher, &QDBusServiceWatcher::serviceRegistered, this, &ObexManagerPrivate::serviceRegistered);
    connect(serviceWatcher, &QDBusServiceWatcher::serviceUnregistered, this, &ObexManagerPrivate::serviceUnregistered);

    // Update the current state of org.bluez.obex service
    if (!DBusConnection::orgBluezObex().isConnected()) {
        Q_EMIT initError(QStringLiteral("DBus session bus is not connected!"));
        return;
    }

    QDBusMessage call =
        QDBusMessage::createMethodCall(Strings::orgFreedesktopDBus(), QStringLiteral("/"), Strings::orgFreedesktopDBus(), QStringLiteral("NameHasOwner"));

    call << Strings::orgBluezObex();

    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(DBusConnection::orgBluezObex().asyncCall(call));
    connect(watcher, &QDBusPendingCallWatcher::finished, this, &ObexManagerPrivate::nameHasOwnerFinished);
}

void ObexManagerPrivate::nameHasOwnerFinished(QDBusPendingCallWatcher *watcher)
{
    const QDBusPendingReply<bool> &reply = *watcher;
    watcher->deleteLater();

    if (reply.isError()) {
        Q_EMIT initError(reply.error().message());
        return;
    }

    m_obexRunning = reply.value();

    if (m_obexRunning) {
        load();
    } else {
        m_initialized = true;
        Q_EMIT initFinished();
    }
}

void ObexManagerPrivate::load()
{
    if (!m_obexRunning || m_loaded) {
        return;
    }

    // Force QDBus to cache owner of org.bluez.obex - this will be the only blocking call on session connection
    DBusConnection::orgBluezObex()
        .connect(Strings::orgBluezObex(), QStringLiteral("/"), Strings::orgFreedesktopDBus(), QStringLiteral("Dummy"), this, SLOT(dummy()));

    m_dbusObjectManager = new DBusObjectManager(Strings::orgBluezObex(), QStringLiteral("/"), DBusConnection::orgBluezObex(), this);

    connect(m_dbusObjectManager, &DBusObjectManager::InterfacesAdded, this, &ObexManagerPrivate::interfacesAdded);
    connect(m_dbusObjectManager, &DBusObjectManager::InterfacesRemoved, this, &ObexManagerPrivate::interfacesRemoved);

    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(m_dbusObjectManager->GetManagedObjects(), this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, &ObexManagerPrivate::getManagedObjectsFinished);
}

void ObexManagerPrivate::getManagedObjectsFinished(QDBusPendingCallWatcher *watcher)
{
    const QDBusPendingReply<DBusManagerStruct> &reply = *watcher;
    watcher->deleteLater();

    if (reply.isError()) {
        Q_EMIT initError(reply.error().message());
        return;
    }

    DBusManagerStruct::const_iterator it;
    const DBusManagerStruct &managedObjects = reply.value();

    for (it = managedObjects.constBegin(); it != managedObjects.constEnd(); ++it) {
        const QString &path = it.key().path();
        const QVariantMapMap &interfaces = it.value();

        if (interfaces.contains(Strings::orgBluezObexSession1())) {
            addSession(path, interfaces.value(Strings::orgBluezObexSession1()));
        } else if (interfaces.contains(Strings::orgBluezObexClient1()) && interfaces.contains(Strings::orgBluezObexAgentManager1())) {
            m_obexClient = new ObexClient(Strings::orgBluezObex(), path, DBusConnection::orgBluezObex(), this);
            m_obexAgentManager = new ObexAgentManager(Strings::orgBluezObex(), path, DBusConnection::orgBluezObex(), this);
        }
    }

    if (!m_obexClient) {
        Q_EMIT initError(QStringLiteral("Cannot find org.bluez.obex.Client1 object!"));
        return;
    }

    if (!m_obexAgentManager) {
        Q_EMIT initError(QStringLiteral("Cannot find org.bluez.obex.AgentManager1 object!"));
        return;
    }

    m_loaded = true;
    m_initialized = true;

    Q_EMIT q->operationalChanged(true);
    Q_EMIT initFinished();
}

void ObexManagerPrivate::clear()
{
    m_loaded = false;

    // Delete all sessions
    while (!m_sessions.isEmpty()) {
        ObexSessionPtr session = m_sessions.begin().value();
        m_sessions.remove(m_sessions.begin().key());
        Q_EMIT q->sessionRemoved(session);
    }

    // Delete all other objects
    if (m_obexClient) {
        m_obexClient->deleteLater();
        m_obexClient = nullptr;
    }

    if (m_obexAgentManager) {
        m_obexAgentManager->deleteLater();
        m_obexAgentManager = nullptr;
    }

    if (m_dbusObjectManager) {
        m_dbusObjectManager->deleteLater();
        m_dbusObjectManager = nullptr;
    }
}

void ObexManagerPrivate::serviceRegistered()
{
    qCDebug(BLUEZQT) << "Obex service registered";
    m_obexRunning = true;

    // Client1 and AgentManager1 objects are not ready by the time org.bluez.obex is registered
    // nor will the ObjectManager emits interfacesAdded for adding them...
    // So we delay the call to load() by 0.5s
    m_timer.start(500);
}

void ObexManagerPrivate::serviceUnregistered()
{
    qCDebug(BLUEZQT) << "Obex service unregistered";
    m_obexRunning = false;

    clear();
    Q_EMIT q->operationalChanged(false);
}

void ObexManagerPrivate::interfacesAdded(const QDBusObjectPath &objectPath, const QVariantMapMap &interfaces)
{
    const QString &path = objectPath.path();
    QVariantMapMap::const_iterator it;

    for (it = interfaces.constBegin(); it != interfaces.constEnd(); ++it) {
        if (it.key() == Strings::orgBluezObexSession1()) {
            addSession(path, it.value());
        }
    }
}

void ObexManagerPrivate::interfacesRemoved(const QDBusObjectPath &objectPath, const QStringList &interfaces)
{
    const QString &path = objectPath.path();

    for (const QString &interface : interfaces) {
        if (interface == Strings::orgBluezObexSession1()) {
            removeSession(path);
        }
    }
}

void ObexManagerPrivate::addSession(const QString &sessionPath, const QVariantMap &properties)
{
    ObexSessionPtr session = ObexSessionPtr(new ObexSession(sessionPath, properties));
    session->d->q = session.toWeakRef();
    m_sessions.insert(sessionPath, session);

    Q_EMIT q->sessionAdded(session);
}

void ObexManagerPrivate::removeSession(const QString &sessionPath)
{
    ObexSessionPtr session = m_sessions.take(sessionPath);
    if (!session) {
        return;
    }

    Q_EMIT q->sessionRemoved(session);
}

void ObexManagerPrivate::dummy()
{
}

} // namespace BluezQt

#include "moc_obexmanager_p.cpp"
