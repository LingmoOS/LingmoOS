/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * SPDX-FileCopyrightText: 2014-2015 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef BLUEZQT_OBEXMANAGER_P_H
#define BLUEZQT_OBEXMANAGER_P_H

#include <QObject>
#include <QTimer>

#include "dbusobjectmanager.h"
#include "obexagentmanager1.h"
#include "obexclient1.h"
#include "types.h"

namespace BluezQt
{
typedef org::bluez::obex::Client1 ObexClient;
typedef org::bluez::obex::AgentManager1 ObexAgentManager;
typedef org::freedesktop::DBus::ObjectManager DBusObjectManager;

class ObexManager;

class ObexManagerPrivate : public QObject
{
    Q_OBJECT

public:
    explicit ObexManagerPrivate(ObexManager *qq);

    void init();
    void nameHasOwnerFinished(QDBusPendingCallWatcher *watcher);
    void load();
    void getManagedObjectsFinished(QDBusPendingCallWatcher *watcher);
    void clear();

    void serviceRegistered();
    void serviceUnregistered();
    void interfacesAdded(const QDBusObjectPath &objectPath, const QVariantMapMap &interfaces);
    void interfacesRemoved(const QDBusObjectPath &objectPath, const QStringList &interfaces);

    void addSession(const QString &sessionPath, const QVariantMap &properties);
    void removeSession(const QString &sessionPath);

    ObexManager *q;
    ObexClient *m_obexClient;
    ObexAgentManager *m_obexAgentManager;
    DBusObjectManager *m_dbusObjectManager;

    QTimer m_timer;
    QHash<QString, ObexSessionPtr> m_sessions;

    bool m_initialized;
    bool m_obexRunning;
    bool m_loaded;

Q_SIGNALS:
    void initError(const QString &errorText);
    void initFinished();

private Q_SLOTS:
    void dummy();
};

} // namespace BluezQt

#endif // BLUEZQT_OBEXMANAGER_P_H
