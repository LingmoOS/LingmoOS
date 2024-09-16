// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "account.h"

#include <QDBusObjectPath>
#include <QDBusArgument>
#include <QDBusMetaType>
#include <QDBusInterface>
#include <QDBusPendingCall>
#include <QDBusPendingReply>
#include <QDBusServiceWatcher>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

#define LOGIN1SERVICE "org.freedesktop.login1"
#define LOGIN1MANAGERINTERFACE "org.freedesktop.login1.Session"

#define LOCKERVICE "com.deepin.dde.LockService"
#define LOCKPATH "/com/deepin/dde/LockService"
#define LOCKINTERFACE "com.deepin.dde.LockService"

#define SESSIONMANAGERSERVICE "com.deepin.SessionManager"
#define SESSIONMANAGERPATH "/com/deepin/SessionManager"
#define SESSIONMANAGERINTERFACE "com.deepin.SessionManager"

#define IAM_SERVICE "com.deepin.udcp.iam"
#define IAM_PATH "/com/deepin/udcp/iam"
#define IAM_INTERFACE "com.deepin.udcp.iam"

using namespace accountnetwork::sessionservice;

Account::Account(QObject *parent)
    : QObject (parent)
    , m_isIam(false)
{
    initAccount();
    initActiveAccount();
}

Account::~Account() = default;

QString Account::name() const
{
    return m_accountName;
}

QString Account::activeAccount() const
{
    return m_activeAccountName;
}

void Account::initAccount()
{
    // 获取当前的用户
    auto getAccountName = [](bool &isIam) {
        QDBusInterface dbusInter(SESSIONMANAGERSERVICE, SESSIONMANAGERPATH, SESSIONMANAGERINTERFACE, QDBusConnection::sessionBus());
        QString sessionPath = dbusInter.property("CurrentSessionPath").value<QDBusObjectPath>().path();
        QString currentUid = dbusInter.property("CurrentUid").toString();

        isIam = false;
        QDBusInterface iamInter(IAM_SERVICE, IAM_PATH, IAM_INTERFACE, QDBusConnection::systemBus());
        QDBusPendingCall pendingReply = iamInter.asyncCall("GetUserIdList");
        pendingReply.waitForFinished();
        QDBusPendingReply<QList<uint32_t>> reply = pendingReply.reply();
        QList<uint32_t> userIds = reply.value();
        for (uint32_t uid: userIds) {
            if (QString::number(uid) == currentUid) {
                isIam = true;
                break;
            }
        }

        QDBusInterface login1Inter(LOGIN1SERVICE, sessionPath, LOGIN1MANAGERINTERFACE, QDBusConnection::systemBus());
        return login1Inter.property("Name").toString();
    };
    if (QDBusConnection::sessionBus().interface()->isServiceRegistered(SESSIONMANAGERINTERFACE)) {
        m_accountName = getAccountName(m_isIam);
    } else {
        QDBusServiceWatcher *serviceWatcher = new QDBusServiceWatcher(this);
        serviceWatcher->setConnection(QDBusConnection::sessionBus());
        serviceWatcher->addWatchedService(SESSIONMANAGERINTERFACE);
        connect(serviceWatcher, &QDBusServiceWatcher::serviceRegistered, this, [ this, getAccountName ] {
            m_accountName = getAccountName(m_isIam);
        });
    }
}

void Account::initActiveAccount()
{
    QDBusServiceWatcher *serviceWatcher = new QDBusServiceWatcher(this);
    serviceWatcher->setConnection(QDBusConnection::systemBus());
    serviceWatcher->addWatchedService(LOCKINTERFACE);
    connect(serviceWatcher, &QDBusServiceWatcher::serviceRegistered, this, [ this ] {
        loadActiveAccount();
    });

    if (QDBusConnection::systemBus().interface()->isServiceRegistered(LOCKERVICE)) {
        loadActiveAccount();
    } else {
        QDBusInterface interface("org.freedesktop.DBus", "/org/freedesktop/DBus", "org.freedesktop.DBus", QDBusConnection::systemBus(), this);
        interface.asyncCallWithArgumentList("StartServiceByName", QList<QVariant>() << LOCKINTERFACE << static_cast<uint>(0));
    }
    QDBusConnection::systemBus().connect(LOCKERVICE, LOCKPATH, LOCKINTERFACE, "UserChanged", this, SLOT(onAccountChanged(const QString &)));
}

void Account::loadActiveAccount()
{
    QDBusInterface dbusInter(LOCKERVICE, LOCKPATH, LOCKINTERFACE, QDBusConnection::systemBus());
    QDBusPendingCall reply = dbusInter.asyncCall("CurrentUser");
    reply.waitForFinished();
    QDBusPendingReply<QString> replyResult = reply.reply();
    m_activeAccountName = parseAccount(replyResult.value());
}

QString Account::parseAccount(const QString &accountInfo)
{
    QJsonDocument json = QJsonDocument::fromJson(accountInfo.toLocal8Bit());
    if (!json.isObject())
        return QString();

    return json.object().value("Name").toString();
}

bool Account::isIam()
{
    return m_isIam;
}

void Account::onAccountChanged(const QString &username)
{
    m_activeAccountName = parseAccount(username);
    qDebug() << "current account changed" << m_activeAccountName;
}
