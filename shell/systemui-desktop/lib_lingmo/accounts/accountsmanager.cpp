/****************************************************************************
 * This file is part of Qt AccountsService.
 *
 * Copyright (C) 2018 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * $BEGIN_LICENSE:LGPLv3+$
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * $END_LICENSE$
 ***************************************************************************/

#include <QtCore/QDebug>
#include <QtDBus/QDBusPendingCallWatcher>

#include "accountsmanager.h"
#include "accountsmanager_p.h"

namespace QtAccountsService {

/*
 * AccountsManagerPrivate
 */

AccountsManagerPrivate::AccountsManagerPrivate(const QDBusConnection &bus)
{
    interface =
        new OrgFreedesktopAccountsInterface(QStringLiteral("org.freedesktop.Accounts"),
                                            QStringLiteral("/org/freedesktop/Accounts"), bus);
}

AccountsManagerPrivate::~AccountsManagerPrivate()
{
    qDeleteAll(usersCache);
    delete interface;
}

void AccountsManagerPrivate::_q_userAdded(const QDBusObjectPath &path)
{
    Q_Q(AccountsManager);

    if (usersCache.contains(path.path())) {
        Q_EMIT q->userAdded(usersCache[path.path()]);
        return;
    }

    UserAccount *account = new UserAccount(path.path(), interface->connection());
    usersCache[path.path()] = account;
    Q_EMIT q->userAdded(account);
}

void AccountsManagerPrivate::_q_userDeleted(const QDBusObjectPath &path)
{
    Q_Q(AccountsManager);

    UserAccount *account = usersCache.value(path.path(), nullptr);
    if (!account)
        account = new UserAccount(path.path());
    usersCache.remove(path.path());
    Q_EMIT q->userDeleted(account->userId());
    account->deleteLater();
}

/*!
    \class AccountsManager
    \brief AccountsManager provides the backend for user management.

    The AccountsManager class allows for querying and manipiulating user
    account information.

    \author Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
*/

/*!
    Constructs a AccountsManager object.
*/
AccountsManager::AccountsManager(const QDBusConnection &bus, QObject *parent)
    : QObject(parent)
    , d_ptr(new AccountsManagerPrivate(bus))
{
    qRegisterMetaType<UserAccount::AccountType>("UserAccount::AccountType");
    qRegisterMetaType<UserAccount *>("UserAccount*");

    d_ptr->q_ptr = this;

    connect(d_ptr->interface, SIGNAL(UserAdded(QDBusObjectPath)), // clazy:exclude=old-style-connect
            this, SLOT(_q_userAdded(QDBusObjectPath)));
    connect(d_ptr->interface,
            SIGNAL(UserDeleted(QDBusObjectPath)), // clazy:exclude=old-style-connect
            this, SLOT(_q_userDeleted(QDBusObjectPath)));
}

/*!
    Destroy a AccountsManager object.
*/
AccountsManager::~AccountsManager()
{
    delete d_ptr;
}

/*!
    Caches a user account, so that it shows up in listCachedUsers() output.
    The user name may be a remote user, but the system must be able to lookup
    the user name and resolve the user information.

    A userCached() signal with a UserAccount pointer will be emitted as soon
    as the user account has been cached by AccountsService.

    \param userName The user name for the user.
*/
void AccountsManager::cacheUser(const QString &userName)
{
    Q_D(AccountsManager);

    QDBusPendingCall call = d->interface->CacheUser(userName);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [=](QDBusPendingCallWatcher *w) {
        QDBusPendingReply<QDBusObjectPath> reply = *w;
        w->deleteLater();
        if (reply.isError()) {
            QDBusError error = reply.error();
            qWarning("Couldn't cache user %s: %s", userName.toUtf8().constData(),
                     error.errorString(error.type()).toUtf8().constData());
        } else {
            QDBusObjectPath path = reply.argumentAt<0>();
            if (path.path().isEmpty())
                return;

            UserAccount *account = d->usersCache.value(path.path(), nullptr);
            if (!account) {
                account = new UserAccount(path.path(), d->interface->connection());
                d->usersCache[path.path()] = account;
            }
            Q_EMIT userCached(userName);
        }
    });
}

/*!
    Releases all metadata about a user account, including icon, language and session.
    If the user account is from a remote server and the user has never logged in
    before, then that account will no longer show up in listCachedUsers() output.

    Emits userUncached() when done.

    \param userName The user name for the user.
*/
void AccountsManager::uncacheUser(const QString &userName)
{
    Q_D(AccountsManager);

    QDBusPendingCall call = d->interface->UncacheUser(userName);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [=](QDBusPendingCallWatcher *w) {
        QDBusPendingReply<QDBusObjectPath> reply = *w;
        w->deleteLater();
        if (reply.isError()) {
            QDBusError error = reply.error();
            qWarning("Couldn't uncache user %s: %s", userName.toUtf8().constData(),
                     error.errorString(error.type()).toUtf8().constData());
        } else {
            QMap<QString, UserAccount *>::iterator it = d->usersCache.begin();
            while (it != d->usersCache.end()) {
                auto account = it.value();
                if (account->userName() == userName) {
                    d->usersCache.erase(++it);
                    account->deleteLater();
                } else {
                    ++it;
                }
            }
            Q_EMIT userUncached(userName);
        }
    });
}

/*!
    List cache users asynchronously.
    When the list is ready, \c listCachedUsersFinished will be emitted.
*/
void AccountsManager::listCachedUsers()
{
    Q_D(AccountsManager);

    QDBusPendingCall call = d->interface->ListCachedUsers();
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [=](QDBusPendingCallWatcher *w) {
        QDBusPendingReply<QList<QDBusObjectPath>> reply = *w;
        w->deleteLater();
        if (reply.isError()) {
            QDBusError error = reply.error();
            qWarning("Couldn't list cached users: %s",
                     error.errorString(error.type()).toUtf8().constData());
        } else {
            UserAccountList userList;

            QList<QDBusObjectPath> value = reply.argumentAt<0>();
            userList.reserve(value.size());
            for (int i = 0; i < value.size(); i++) {
                const QString path = value.at(i).path();
                UserAccount *account = d->usersCache.value(path, nullptr);
                if (!account) {
                    account = new UserAccount(path, d->interface->connection());
                    d->usersCache[path] = account;
                }
                userList.append(account);
            }
            Q_EMIT listCachedUsersFinished(userList);
        }
    });
}

/*!
    Returns a list of user accounts.

    This method is synchronous and may take some time.

    \param systemUsers If true, returns also system users.
*/
UserAccountList AccountsManager::listCachedUsersSync()
{
    Q_D(AccountsManager);

    UserAccountList list;

    QDBusPendingReply<QList<QDBusObjectPath>> reply = d->interface->ListCachedUsers();
    reply.waitForFinished();

    if (reply.isError()) {
        QDBusError error = reply.error();
        qWarning("Couldn't list cached users: %s",
                 error.errorString(error.type()).toUtf8().constData());
        return list;
    }

    QList<QDBusObjectPath> value = reply.argumentAt<0>();
    list.reserve(value.size());

    for (int i = 0; i < value.size(); i++) {
        const QString path = value.at(i).path();
        UserAccount *account = d->usersCache.value(path, nullptr);
        if (!account) {
            account = new UserAccount(path, d->interface->connection());
            d->usersCache[path] = account;
        }
        list.append(account);
    }

    return list;
}

/*!
    Returns user with name \a userName from the list of cached users
    created by \l listCachedUsers().

    \param userName The user name to look up.
    \return the corresponding UserAccount object.
*/
UserAccount *AccountsManager::cachedUser(const QString &userName) const
{
    Q_D(const AccountsManager);

    QMap<QString, UserAccount *>::const_iterator it = d->usersCache.constBegin();
    while (it != d->usersCache.constEnd()) {
        auto account = it.value();
        if (account->userName() == userName)
            return account;
    }

    return nullptr;
}

/*!
    Finds a user by \a uid.

    This method is synchronous and may take some time.

    \param uid The uid to look up.
    \return the corresponding UserAccount object.
*/
UserAccount *AccountsManager::findUserById(qlonglong uid)
{
    Q_D(AccountsManager);

    QDBusPendingReply<QDBusObjectPath> reply = d->interface->FindUserById(uid);
    reply.waitForFinished();

    if (reply.isError()) {
        QDBusError error = reply.error();
        qWarning("Couldn't find user by uid %lld: %s", uid,
                 error.errorString(error.type()).toUtf8().constData());
        return nullptr;
    }

    QDBusObjectPath path = reply.argumentAt<0>();
    if (path.path().isEmpty())
        return nullptr;

    UserAccount *account = d->usersCache.value(path.path(), nullptr);
    if (!account) {
        account = new UserAccount(path.path(), d->interface->connection());
        d->usersCache[path.path()] = account;
    }
    return account;
}

/*!
    Finds a user by user \a userName.

    This method is synchronous and may take some time.

    \param uid The user name to look up.
    \return the corresponding UserAccount object.
*/
UserAccount *AccountsManager::findUserByName(const QString &userName)
{
    Q_D(AccountsManager);

    QDBusPendingReply<QDBusObjectPath> reply = d->interface->FindUserByName(userName);
    reply.waitForFinished();

    if (reply.isError()) {
        QDBusError error = reply.error();
        qWarning("Couldn't find user by user name %s: %s", userName.toUtf8().constData(),
                 error.errorString(error.type()).toUtf8().constData());
        return nullptr;
    }

    QDBusObjectPath path = reply.argumentAt<0>();
    if (path.path().isEmpty())
        return nullptr;

    UserAccount *account = d->usersCache.value(path.path(), nullptr);
    if (!account) {
        account = new UserAccount(path.path(), d->interface->connection());
        d->usersCache[path.path()] = account;
    }
    return account;
}

/*!
    Creates a new \a accountType type user account whose name is \a userName,
    real name is \a fullName.

    This method is synchronous and may take some time.

    \param userName The name of the new user to be created.
    \param fullName First name and last name.
    \param accountType The account type.
    \return whether the user was created successfully.
*/
bool AccountsManager::createUser(const QString &userName, const QString &fullName,
                                 UserAccount::AccountType accountType)
{
    Q_D(AccountsManager);

    QDBusPendingReply<QDBusObjectPath> reply =
        d->interface->CreateUser(userName, fullName, accountType);
    if (reply.isError()) {
        QDBusError error = reply.error();
        qWarning("Couldn't create user %s: %s", userName.toUtf8().constData(),
                 error.errorString(error.type()).toUtf8().constData());
        return false;
    }

    return true;
}

/*!
    Deletes the user designated by \a uid.

    This method is synchronous and may take some time.

    \param uid The user identifier.
    \param removeFiles If true all files owned by the user will be removed.
    \return whether the user was deleted successfully.
*/
bool AccountsManager::deleteUser(qlonglong uid, bool removeFiles)
{
    Q_D(AccountsManager);

    QDBusPendingReply<> reply = d->interface->DeleteUser(uid, removeFiles);
    if (reply.isError()) {
        QDBusError error = reply.error();
        qWarning("Couldn't delete user %lld: %s", uid,
                 error.errorString(error.type()).toUtf8().constData());
        return false;
    }

    return true;
}

/*!
    Deletes the specified \a account.

    This method is synchronous and may take some time.

    \param account The account to remove.
    \param removeFiles If true all files owned by the user will be removed.
    \return whether the user was deleted successfully.
*/
bool AccountsManager::deleteUser(UserAccount *account, bool removeFiles)
{
    return deleteUser(account->userId(), removeFiles);
}
}

#include "moc_accountsmanager.cpp"
