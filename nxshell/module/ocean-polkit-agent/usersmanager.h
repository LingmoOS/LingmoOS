// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef USERSMANAGER_H
#define USERSMANAGER_H

#include <QObject>
#include <QMap>
#include <QString>

#include "accounts1interface.h"
#include "accounts1userinterface.h"

using AccountsDBus = org::lingmo::ocean::Accounts1;
using UserDBus = org::lingmo::ocean::accounts1::User;

class UsersManager : public QObject
{
    Q_OBJECT
public:
    static UsersManager *instance();

    QString getFullName(const QString &name);

private slots:
    void userAoceand(const QString &userPath);
    void userRemoved(const QString &userPath);

private:
    explicit UsersManager(QObject *parent = nullptr);

private:
    QMap<QString, QString> m_fullNameMap;
    QMap<QString, UserDBus*> m_usersMap;
    AccountsDBus *m_accountsInter;
};

#endif // USERSMANAGER_H
