// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "usersmanager.h"

static UsersManager *userManagerInstance = Q_NULLPTR;

UsersManager::UsersManager(QObject *parent)
    : QObject(parent)
    , m_accountsInter(new AccountsDBus("com.deepin.dde.Accounts1",
                                       "/com/deepin/dde/Accounts1",
                                       QDBusConnection::systemBus(),
                                       this))
{
    const QStringList userList = m_accountsInter->userList();
    for (const QString &userPath : userList) {
        userAdded(userPath);
    }

    connect(m_accountsInter, &AccountsDBus::UserAdded, this, &UsersManager::userAdded);
    connect(m_accountsInter, &AccountsDBus::UserDeleted, this, &UsersManager::userRemoved);
}

UsersManager *UsersManager::instance()
{
    if (!userManagerInstance)
        userManagerInstance = new UsersManager;

    return userManagerInstance;
}

QString UsersManager::getFullName(const QString &name)
{
    return m_fullNameMap.value(name);
}

void UsersManager::userAdded(const QString &userPath)
{
    UserDBus *userInter = new UserDBus("org.deepin.dde.Accounts1",
                                       userPath,
                                       QDBusConnection::systemBus(),
                                       this);
    const QString &username = userInter->userName();

    m_usersMap[userPath] = userInter;
    m_fullNameMap[username] = userInter->fullName();

    connect(userInter, &__AccountsUser::FullNameChanged, this, [this, username] (const QString &fullname) {
        m_fullNameMap[username]  = fullname;
    });
}

void UsersManager::userRemoved(const QString &userPath)
{
    m_usersMap.remove(userPath);
}
