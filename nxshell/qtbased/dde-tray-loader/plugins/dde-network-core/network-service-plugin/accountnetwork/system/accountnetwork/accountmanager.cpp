// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "accountmanager.h"
#include "networkconfig.h"

#include <QDBusObjectPath>
#include <QDBusArgument>
#include <QDBusMetaType>
#include <QDBusInterface>
#include <QDBusPendingCall>
#include <QDBusPendingReply>
#include <QDBusConnectionInterface>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDBusServiceWatcher>

using namespace accountnetwork::systemservice;

#define LOCKERVICE "com.deepin.dde.LockService"
#define LOCKPATH "/com/deepin/dde/LockService"
#define LOCKINTERFACE "com.deepin.dde.LockService"

#define ACCOUNT_SERVICE "com.deepin.daemon.Accounts"
#define ACCOUNT_PATH "/com/deepin/daemon/Accounts"
#define ACCOUNT_INTERFACE "com.deepin.daemon.Accounts"

AccountManager::AccountManager(NetworkConfig *conf, QObject *parent)
    : QObject (parent)
    , m_networkConfig(conf)
{
    QDBusServiceWatcher *serviceWatcher = new QDBusServiceWatcher(this);
    serviceWatcher->setConnection(QDBusConnection::systemBus());
    serviceWatcher->addWatchedService(LOCKINTERFACE);
    connect(serviceWatcher, &QDBusServiceWatcher::serviceRegistered, this, [ this ] {
        initAccount();
    });

    if (QDBusConnection::systemBus().interface()->isServiceRegistered(LOCKERVICE)) {
        initAccount();
    } else {
        QDBusInterface interface("org.freedesktop.DBus", "/org/freedesktop/DBus", "org.freedesktop.DBus", QDBusConnection::systemBus(), this);
        interface.asyncCallWithArgumentList("StartServiceByName", QList<QVariant>() << LOCKINTERFACE << static_cast<uint>(0));
    }
    QDBusConnection::systemBus().connect(LOCKERVICE, LOCKPATH, LOCKINTERFACE, "UserChanged", this, SLOT(onAccountChanged(const QString &)));
    QDBusConnection::systemBus().connect(ACCOUNT_SERVICE, ACCOUNT_PATH, ACCOUNT_INTERFACE, "UserAdded", this, SLOT(onUserAdded(const QString &)));
    QDBusConnection::systemBus().connect(ACCOUNT_SERVICE, ACCOUNT_PATH, ACCOUNT_INTERFACE, "UserDeleted", this, SLOT(onUserDeleted(const QString &)));
    QDBusInterface dbusInter(ACCOUNT_SERVICE, ACCOUNT_PATH, ACCOUNT_INTERFACE, QDBusConnection::systemBus());
    QStringList userLists = dbusInter.property("UserList").toStringList();
    for (const QString &user : userLists) {
        QDBusInterface userInterface(ACCOUNT_SERVICE, user, "com.deepin.daemon.Accounts.User", QDBusConnection::systemBus());
        m_userMap[user] = userInterface.property("UserName").toString();
    }
}

AccountManager::~AccountManager() = default;

QString AccountManager::account() const
{
    return m_account;
}

QStringList AccountManager::primaryAccount() const
{
    // 这个函数用来获取非域账户（管理员账户），考虑存在多个非域账户的情况，所以这里用QStringList，并返回给外部
    QStringList primaryAccounts;
    QStringList iamUsers = iamUserList();
    QDBusInterface dbusInter(ACCOUNT_SERVICE, ACCOUNT_PATH, ACCOUNT_INTERFACE, QDBusConnection::systemBus());
    QStringList userLists = dbusInter.property("UserList").toStringList();
    for (const QString &userPath : userLists) {
        QDBusInterface interface(ACCOUNT_SERVICE, userPath, "com.deepin.daemon.Accounts.User", QDBusConnection::systemBus());
        if (iamUsers.contains(interface.property("UserName").toString()))
            continue;

        primaryAccounts << interface.property("UserName").toString();
    }

    return primaryAccounts;
}

bool AccountManager::accountExist(const QString &accountname) const
{
    QDBusInterface dbusInter(ACCOUNT_SERVICE, ACCOUNT_PATH, ACCOUNT_INTERFACE, QDBusConnection::systemBus());
    QDBusPendingCall reply = dbusInter.asyncCall("FindUserByName", accountname);
    reply.waitForFinished();
    if (reply.isError())
        return false;

    QDBusPendingReply<QString> replyResult = reply.reply();
    return (!replyResult.value().isEmpty());
}

void AccountManager::initAccount()
{
    QDBusInterface dbusInter(LOCKERVICE, LOCKPATH, LOCKINTERFACE, QDBusConnection::systemBus());
    QDBusPendingCall reply = dbusInter.asyncCall("CurrentUser");
    reply.waitForFinished();
    QDBusPendingReply<QString> replyResult = reply.reply();
    m_account = parseAccount(replyResult.value());
}

QString AccountManager::parseAccount(const QString &accountInfo)
{
    QJsonDocument json = QJsonDocument::fromJson(accountInfo.toLocal8Bit());
    if (!json.isObject())
        return QString();

    return json.object().value("Name").toString();
}

QStringList AccountManager::iamUserList() const
{
    QDBusInterface iamInterface("com.deepin.udcp.iam","/com/deepin/udcp/iam","com.deepin.udcp.iam", QDBusConnection::systemBus());
    QDBusPendingReply<QList<uint32_t>> iamUidsReply = iamInterface.asyncCall("GetUserIdList");
    QList<uint32_t> iamUids = iamUidsReply.value();
    QStringList allIamUids;
    for (uint32_t uid : iamUids) {
        allIamUids << QString::number(uid);
    }
    return allIamUids;
}

void AccountManager::onAccountChanged(const QString &username)
{
    m_account = parseAccount(username);
    emit accountChanged(m_account);
}

void AccountManager::onUserAdded(const QString &path)
{
    // 新增用户的时候，需要告知外部，如果是新增的域账户，外部需要处理
    QDBusInterface interface(ACCOUNT_SERVICE, path, "com.deepin.daemon.Accounts.User", QDBusConnection::systemBus());
    QString userName = interface.property("UserName").toString();
    m_userMap[path] = userName;
    emit accountAdded(userName, iamUserList().contains(QString::number(interface.property("Uid").toUInt())));
}

void AccountManager::onUserDeleted(const QString &path)
{
    if (!m_userMap.contains(path))
        return;

    QString username = m_userMap.value(path);
    m_userMap.remove(path);
    emit accountRemoved(username);
}
