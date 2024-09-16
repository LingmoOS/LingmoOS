// SPDX-FileCopyrightText: 2022 Uniontech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dsystemaccountsinterface.h"

#include <qdebug.h>

DACCOUNTS_BEGIN_NAMESPACE

DSystemAccountsInterface::DSystemAccountsInterface(QObject *parent)
    : QObject(parent)
{
#if defined(USE_FAKE_INTERFACE)
    const QString &Service = QStringLiteral("com.deepin.daemon.FakeAccounts");
    const QString &Path = QStringLiteral("/com/deepin/daemon/FakeAccounts");
    const QString &Interface = QStringLiteral("com.deepin.daemon.FakeAccounts");
    QDBusConnection Connection = QDBusConnection::sessionBus();
#else
    const QString &Service = QStringLiteral("org.deepin.dde.Accounts1");
    const QString &Path = QStringLiteral("/org/deepin/dde/Accounts1");
    const QString &Interface = QStringLiteral("org.deepin.dde.Accounts1");
    QDBusConnection Connection = QDBusConnection::systemBus();
    Connection
            .connect(Service, Path, Interface, "UserAdded", this, SLOT(receiveUserAdded(QString)));
    Connection.connect(Service,
                       Path,
                       Interface,
                       "UserDeleted",
                       this,
                       SLOT(receiveUserDeleted(QString)));
#endif
    m_inter = new DDBusInterface(Service, Path, Interface, Connection, this);
};

QDBusPendingReply<QDBusObjectPath> DSystemAccountsInterface::createUser(const QString &name,
                                                                        const QString &fullName,
                                                                        qint32 accountType)
{
    return m_inter->asyncCallWithArgumentList("CreateUser",
                                              { QVariant::fromValue(name),
                                                QVariant::fromValue(fullName),
                                                QVariant::fromValue(accountType) });
}

QDBusPendingReply<void> DSystemAccountsInterface::deleteUser(const QString &name, bool rmFiles)
{
    return m_inter->asyncCallWithArgumentList(
            "DeleteUser",
            { QVariant::fromValue(name), QVariant::fromValue(rmFiles) });
}

QDBusPendingReply<QStringList> DSystemAccountsInterface::getPresetGroups(qint32 accountType)
{
    return m_inter->asyncCallWithArgumentList("GetPresetGroups",
                                              { QVariant::fromValue(accountType) });
}

QDBusPendingReply<bool, QString, qint32>
DSystemAccountsInterface::isPasswordValid(const QString &password)
{
    return m_inter->asyncCallWithArgumentList("IsPasswordValid", { QVariant::fromValue(password) });
}

QDBusPendingReply<bool, QString, qint32>
DSystemAccountsInterface::isUsernameValid(const QString &username)
{
    return m_inter->asyncCallWithArgumentList("IsUsernameValid", { QVariant::fromValue(username) });
}

QStringList DSystemAccountsInterface::UserList() const
{
    return qdbus_cast<QStringList>(m_inter->property("UserList"));
}

void DSystemAccountsInterface::receiveUserAdded(QString user)
{
    emit ReceivedUserAdded(std::move(user));
}

void DSystemAccountsInterface::receiveUserDeleted(QString user)
{
    emit ReceivedUserDeleted(std::move(user));
}

DACCOUNTS_END_NAMESPACE
