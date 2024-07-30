/*
 *  SPDX-FileCopyrightText: 2013 Alejandro Fiestas Fiestas <afiestas@kde.org>
 *  SPDX-FileCopyrightText: 2014-2015 David Rosca <nowrep@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <QDBusAbstractAdaptor>
#include <QDBusMessage>
#include <QHash>

#include <BluezQt/Manager>

class QDBusPendingCallWatcher;

class BlueDevilDaemon;

class Q_DECL_EXPORT ObexFtp : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.BlueDevil.ObexFtp")

public:
    explicit ObexFtp(BlueDevilDaemon *daemon);

    Q_SCRIPTABLE bool isOnline();
    Q_SCRIPTABLE QString preferredTarget(const QString &address);
    Q_SCRIPTABLE QString session(const QString &address, const QString &target, const QDBusMessage &msg);
    Q_SCRIPTABLE bool cancelTransfer(const QString &transfer, const QDBusMessage &msg);

private Q_SLOTS:
    void createSessionFinished(BluezQt::PendingCall *call);
    void cancelTransferFinished(QDBusPendingCallWatcher *watcher);
    void sessionRemoved(BluezQt::ObexSessionPtr session);

private:
    BlueDevilDaemon *const m_daemon;
    QHash<QString, QString> m_sessionMap;
    QHash<QString, QList<QDBusMessage>> m_pendingSessions;
};
