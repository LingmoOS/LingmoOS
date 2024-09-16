// SPDX-FileCopyrightText: 2022 Uniontech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include "dtkaccounts_global.h"

#include <DDBusInterface>

#include <QDBusPendingReply>

DACCOUNTS_BEGIN_NAMESPACE
using DTK_CORE_NAMESPACE::DDBusInterface;

class DAccountsInterface : public QObject
{
    Q_OBJECT

public:
    explicit DAccountsInterface(QObject *parent = nullptr);
    ~DAccountsInterface() = default;

    Q_PROPERTY(QString daemonVersion READ daemonVersion)
    QString daemonVersion() const;

public slots:
    QDBusPendingReply<QDBusObjectPath> cacheUser(const QString &name);
    QDBusPendingReply<QDBusObjectPath> createUser(const QString &name,
                                                  const QString &fullname,
                                                  const qint32 accountType);
    QDBusPendingReply<void> deleteUser(const qint64 id, const bool removeFiles);
    QDBusPendingReply<QDBusObjectPath> findUserById(const qint64 id);
    QDBusPendingReply<QDBusObjectPath> findUserByName(const QString &name);
    QDBusPendingReply<QList<QDBusObjectPath>> listCachedUsers();
    QDBusPendingReply<void> uncacheUser(const QString &name);

signals:
    void ReceivedUserAdded(QString path);
    void ReceivedUserDeleted(QString path);

private slots:
    void receiveUserAdded(const QDBusObjectPath &user);
    void receiveUserDeleted(const QDBusObjectPath &user);

private:
    DDBusInterface *m_inter;
};

DACCOUNTS_END_NAMESPACE
