// SPDX-FileCopyrightText: 2022 Uniontech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "daccountsinterface.h"

#include <QDBusConnection>

DACCOUNTS_BEGIN_NAMESPACE

DAccountsInterface::DAccountsInterface(QObject *parent)
    : QObject(parent)
{
#if defined(USE_FAKE_INTERFACE)
    const QString &Service = QStringLiteral("com.deepin.daemon.FakeAccounts");
    const QString &Path = QStringLiteral("/com/deepin/daemon/FakeAccounts");
    const QString &Interface = QStringLiteral("com.deepin.daemon.FakeAccounts");
    QDBusConnection Connection = QDBusConnection::sessionBus();
#else
    const QString &Service = QStringLiteral("org.freedesktop.Accounts");
    const QString &Path = QStringLiteral("/org/freedesktop/Accounts");
    const QString &Interface = QStringLiteral("org.freedesktop.Accounts");
    QDBusConnection Connection = QDBusConnection::systemBus();
    Connection.connect(Service,
                       Path,
                       Interface,
                       "UserAdded",
                       this,
                       SLOT(receiveUserAdded(QDBusObjectPath)));
    Connection.connect(Service,
                       Path,
                       Interface,
                       "UserDeleted",
                       this,
                       SLOT(receiveUserDeleted(QDBusObjectPath)));
#endif
    m_inter = new DDBusInterface(Service, Path, Interface, Connection, this);
}

QString DAccountsInterface::daemonVersion() const
{
    return qdbus_cast<QString>(m_inter->property("DaemonVersion"));
}

QDBusPendingReply<QDBusObjectPath> DAccountsInterface::cacheUser(const QString &name)
{
    QVariantList args{ QVariant::fromValue(name) };
    return m_inter->asyncCallWithArgumentList("CacheUser", args);
}

QDBusPendingReply<QDBusObjectPath> DAccountsInterface::createUser(const QString &name,
                                                                  const QString &fullname,
                                                                  const qint32 accountType)
{
    QVariantList args{ QVariant::fromValue(name),
                       QVariant::fromValue(fullname),
                       QVariant::fromValue(accountType) };
    return m_inter->asyncCallWithArgumentList("CreateUser", args);
}

QDBusPendingReply<void> DAccountsInterface::deleteUser(const qint64 id, const bool removeFiles)
{
    QVariantList args{ QVariant::fromValue(id), QVariant::fromValue(removeFiles) };
    auto userPath = findUserById(id);
    return m_inter->asyncCallWithArgumentList("DeleteUser", args);
}

QDBusPendingReply<QDBusObjectPath> DAccountsInterface::findUserById(const qint64 id)
{
    QVariantList args{ QVariant::fromValue(id) };
    return m_inter->asyncCallWithArgumentList("FindUserById", args);
}

QDBusPendingReply<QDBusObjectPath> DAccountsInterface::findUserByName(const QString &name)
{
    QVariantList args{ QVariant::fromValue(name) };
    return m_inter->asyncCallWithArgumentList("FindUserByName", args);
}

QDBusPendingReply<QList<QDBusObjectPath>> DAccountsInterface::listCachedUsers()
{
    return m_inter->asyncCall("ListCachedUsers");
}

QDBusPendingReply<void> DAccountsInterface::uncacheUser(const QString &name)
{
    QVariantList args{ QVariant::fromValue(name) };
    return m_inter->asyncCallWithArgumentList("UncacheUser", args);
}

void DAccountsInterface::receiveUserAdded(const QDBusObjectPath &user)
{
    emit ReceivedUserAdded(user.path().toUtf8());
}

void DAccountsInterface::receiveUserDeleted(const QDBusObjectPath &user)
{
    emit ReceivedUserDeleted(user.path().toUtf8());
}

DACCOUNTS_END_NAMESPACE
