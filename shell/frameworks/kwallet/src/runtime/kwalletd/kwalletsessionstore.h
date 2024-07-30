/*
    This file is part of the KDE Wallet Daemon
    SPDX-FileCopyrightText: 2008 Michael Leupold <lemma@confuego.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef _KWALLETSESSIONSTORE_H_
#define _KWALLETSESSIONSTORE_H_

#include <QHash>
#include <QList>
#include <QPair>
#include <QString>
#include <QStringList>

typedef QPair<QString, int> KWalletAppHandlePair;

class KWalletSessionStore
{
public:
    KWalletSessionStore();
    ~KWalletSessionStore();

    KWalletSessionStore(const KWalletSessionStore &) = delete;
    KWalletSessionStore &operator=(const KWalletSessionStore &) = delete;

    // add a new session
    void addSession(const QString &appid, const QString &service, int handle);
    // check if the application has a session using that handle
    bool hasSession(const QString &appid, int handle = -1) const;
    // find all sessions a service has
    QList<KWalletAppHandlePair> findSessions(const QString &service) const;
    // remove a specific session
    bool removeSession(const QString &appid, const QString &service, int handle);
    // remove all sessions an application has to a specific handle
    int removeAllSessions(const QString &appid, int handle);
    // remove all sessions related to a handle
    int removeAllSessions(int handle);
    // get all applications using a handle
    QStringList getApplications(int handle) const;

private:
    class Session;
    QHash<QString, QList<Session *>> m_sessions; // appid => session
};

#endif // _KWALLETSESSIONSTORE_H_
