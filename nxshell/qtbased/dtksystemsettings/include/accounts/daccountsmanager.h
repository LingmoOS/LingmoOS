// SPDX-FileCopyrightText: 2022 Uniontech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include "daccountstypes.h"

#include <DExpected>
#include <QObject>
#include <QSharedPointer>
#include <QStringList>

DACCOUNTS_BEGIN_NAMESPACE
class DAccountsUser;
DACCOUNTS_END_NAMESPACE

using AccountsUserPtr = QSharedPointer<DTK_ACCOUNTS_NAMESPACE::DAccountsUser>;
using AccountsValidMsg = DTK_ACCOUNTS_NAMESPACE::ValidMsg;
using DAccountTypes = DTK_ACCOUNTS_NAMESPACE::AccountTypes;

DACCOUNTS_BEGIN_NAMESPACE
using DCORE_NAMESPACE::DExpected;

class DAccountsManagerPrivate;

class DAccountsManager : public QObject
{
    Q_OBJECT

public:
    explicit DAccountsManager(QObject *parent = nullptr);
    virtual ~DAccountsManager();

public slots:
    DExpected<QList<quint64>>
    userList() const; // TODO:创建或删除 userlist 无法及时刷新，调用 sleep(1) 正常
    DExpected<AccountsUserPtr> createUser(const QString &name,
                                          const QString &fullName,
                                          const DAccountTypes &type);
    DExpected<void> deleteUser(const QString &name, const bool rmFiles);
    DExpected<AccountsUserPtr> findUserByName(const QString &name);
    DExpected<AccountsUserPtr> findUserById(const qint64 uid);
    DExpected<QStringList> groups();
    DExpected<QStringList> presetGroups(const DAccountTypes &type);
    DExpected<AccountsValidMsg> isPasswordValid(const QString &password);
    DExpected<AccountsValidMsg> isUsernameValid(const QString &name);

signals:
    void UserAdded(const quint64 uid);
    void UserDeleted(const quint64 uid);

private:
    QScopedPointer<DAccountsManagerPrivate> d_ptr;
    Q_DECLARE_PRIVATE(DAccountsManager)
};

DACCOUNTS_END_NAMESPACE
