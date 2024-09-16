// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef USERSMANAGER_H
#define USERSMANAGER_H

#include <QObject>
#include <QMap>
#include <QString>

#include "account_interface.h"
#include "accountuser_interface.h"

using AccountsDBus = org::deepin::dde::Accounts1;
using UserDBus = org::deepin::dde::accounts1::User;

class UsersManager : public QObject
{
    Q_OBJECT
public:
    static UsersManager *instance();

    QString getFullName(const QString &name);

private slots:
    void userAdded(const QString &userPath);
    void userRemoved(const QString &userPath);

private:
    explicit UsersManager(QObject *parent = nullptr);

private:
    QMap<QString, QString> m_fullNameMap;
    QMap<QString, UserDBus*> m_usersMap;
    AccountsDBus *m_accountsInter;
};

#endif // USERSMANAGER_H
