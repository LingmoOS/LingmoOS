// SPDX-FileCopyrightText: 2022 Uniontech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include "daccountstypes.h"

#include <DDBusInterface>

#include <QDBusPendingReply>
#include <QScopedPointer>
#include <QStringList>

DACCOUNTS_BEGIN_NAMESPACE
using DTK_CORE_NAMESPACE::DDBusInterface;

class DSystemAccountsInterface : public QObject
{
    Q_OBJECT

public:
    explicit DSystemAccountsInterface(QObject *parent = nullptr);
    ~DSystemAccountsInterface() = default;

    Q_PROPERTY(QStringList UserList READ UserList NOTIFY UserListChanged)
    QStringList UserList() const;

public slots:
    QDBusPendingReply<QDBusObjectPath> createUser(const QString &name,
                                                  const QString &fullName,
                                                  qint32 accountType);
    QDBusPendingReply<void> deleteUser(const QString &name, bool rmFiles);
    QDBusPendingReply<QStringList> getPresetGroups(qint32 accountType);
    QDBusPendingReply<bool, QString, qint32> isPasswordValid(const QString &password);
    QDBusPendingReply<bool, QString, qint32> isUsernameValid(const QString &username);

signals:
    void ReceivedUserAdded(QString path);
    void ReceivedUserDeleted(QString path);
    void UserListChanged(const QStringList &list);

private slots:
    void receiveUserAdded(QString user);
    void receiveUserDeleted(QString user);

private:
    DDBusInterface *m_inter;
};

DACCOUNTS_END_NAMESPACE
