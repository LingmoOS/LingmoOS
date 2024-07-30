/*
    This file is part of the KDE Wallet Daemon
    SPDX-FileCopyrightText: 2008 Michael Leupold <lemma@confuego.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kwalletsessionstore.h"

class KWalletSessionStore::Session
{
public:
    QString m_service; // client dbus service (or empty)
    int m_handle; // backend handle
};

KWalletSessionStore::KWalletSessionStore()
{
}

KWalletSessionStore::~KWalletSessionStore()
{
    for (const QList<Session *> &l : std::as_const(m_sessions)) {
        qDeleteAll(l);
    }
}

void KWalletSessionStore::addSession(const QString &appid, const QString &service, int handle)
{
    Session *sess = new Session();
    sess->m_service = service;
    sess->m_handle = handle;
    m_sessions[appid].append(sess);
}

bool KWalletSessionStore::hasSession(const QString &appid, int handle) const
{
    if (!m_sessions.contains(appid)) {
        return false;
    } else if (handle == -1) {
        return true;
    }

    QList<Session *>::const_iterator it;
    QList<Session *>::const_iterator end = m_sessions[appid].constEnd();
    for (it = m_sessions[appid].constBegin(); it != end; ++it) {
        Q_ASSERT(*it);
        if ((*it)->m_handle == handle) {
            return true;
        }
    }

    return false;
}

QList<KWalletAppHandlePair> KWalletSessionStore::findSessions(const QString &service) const
{
    QList<KWalletAppHandlePair> rc;
    const QList<QString> sessionKeys(m_sessions.keys());
    for (const QString &appid : sessionKeys) {
        const auto lst = m_sessions[appid];
        for (const Session *sess : lst) {
            Q_ASSERT(sess);
            if (sess->m_service == service) {
                rc.append(qMakePair(appid, sess->m_handle));
            }
        }
    }
    return rc;
}

bool KWalletSessionStore::removeSession(const QString &appid, const QString &service, int handle)
{
    if (!m_sessions.contains(appid)) {
        return false;
    }

    QList<Session *>::const_iterator it;
    QList<Session *>::const_iterator end = m_sessions[appid].constEnd();
    for (it = m_sessions[appid].constBegin(); it != end; ++it) {
        Q_ASSERT(*it);
        if ((*it)->m_service == service && (*it)->m_handle == handle) {
            Session *sess = *it;
            m_sessions[appid].removeAll(sess);
            delete sess;
            if (m_sessions[appid].isEmpty()) {
                m_sessions.remove(appid);
            }
            return true;
        }
    }

    return false;
}

int KWalletSessionStore::removeAllSessions(const QString &appid, int handle)
{
    if (!m_sessions.contains(appid)) {
        return false;
    }

    QList<Session *>::iterator it;
    QList<Session *>::iterator end = m_sessions[appid].end();
    for (it = m_sessions[appid].begin(); it != end; ++it) {
        Q_ASSERT(*it);
        if ((*it)->m_handle == handle) {
            delete *it;
            *it = nullptr;
        }
    }

    int removed = m_sessions[appid].removeAll(nullptr);
    if (m_sessions[appid].isEmpty()) {
        m_sessions.remove(appid);
    }

    return removed;
}

int KWalletSessionStore::removeAllSessions(int handle)
{
    QList<QString> appremove;
    int numrem = 0;
    const QList<QString> sessionKeys(m_sessions.keys());
    for (const QString &appid : sessionKeys) {
        QList<Session *>::iterator it;
        QList<Session *>::iterator end = m_sessions[appid].end();
        for (it = m_sessions[appid].begin(); it != end; ++it) {
            Q_ASSERT(*it);
            if ((*it)->m_handle == handle) {
                delete *it;
                *it = nullptr;
                numrem++;
            }
        }
        // remove all zeroed sessions
        m_sessions[appid].removeAll(nullptr);
        if (m_sessions[appid].isEmpty()) {
            appremove.append(appid);
        }
    }

    // now remove all applications without sessions
    for (const QString &appid : std::as_const(appremove)) {
        m_sessions.remove(appid);
    }

    return numrem;
}

QStringList KWalletSessionStore::getApplications(int handle) const
{
    QStringList rc;
    const auto lst = m_sessions.keys();
    for (const QString &appid : lst) {
        if (hasSession(appid, handle)) {
            rc.append(appid);
        }
    }
    return rc;
}
