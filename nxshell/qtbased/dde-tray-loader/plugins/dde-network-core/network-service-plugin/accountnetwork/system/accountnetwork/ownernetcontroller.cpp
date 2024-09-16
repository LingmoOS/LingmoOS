// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "ownernetcontroller.h"
#include "interfaceserver.h"
#include "networkhandler.h"
#include "accountmanager.h"
#include "networkconfig.h"

#include <QDebug>

using namespace accountnetwork::systemservice;

OwnerNetController::OwnerNetController(QObject *parent)
    : QObject (parent)
    , m_networkConfig(new NetworkConfig(this))
    , m_accountManager(new AccountManager(m_networkConfig, this))
    , m_server(new InterfaceServer(m_accountManager, m_networkConfig, this))
    , m_networkHandler(new NetworkHandler(m_accountManager, m_networkConfig, this))
{
    connect(m_server, &InterfaceServer::requestAuthen, this, &OwnerNetController::onRequestAuthen);
    connect(m_accountManager, &AccountManager::accountChanged, this, &OwnerNetController::onAccountChanged);
    connect(m_accountManager, &AccountManager::accountAdded, this, &OwnerNetController::onAccountAdded);
    connect(m_accountManager, &AccountManager::accountRemoved, this, &OwnerNetController::onAccountRemoved);
}

OwnerNetController::~OwnerNetController() = default;

QVariantMap OwnerNetController::authenInfo() const
{
    return m_server->authenInfo(m_accountManager->account());
}

QMap<QString, QString> OwnerNetController::accountNetwork() const
{
    return m_networkConfig->network(m_accountManager->account());
}

void OwnerNetController::onAccountChanged(const QString &accountname)
{
    qDebug() << "account changed:" << accountname << "previous account:" << accountname;
    if (!m_accountManager->accountExist(accountname))
        return;
    // 存在这样的一种情况，在新增域账户后，输入之前已经存在的域账户的信息，此时会出发账户变化的信号，
    // 之前的域账户的账户名是...，这个时候就找不到...这个账户，因此，需要在账户变化的信号中也做一次处理
    // 由于普通账户不会发送请求认证，在updateIamAuthen函数中会直接不处理，因此，此处无需判断是否为域账户
    // 直接调用即可
    m_server->updateIamAuthen(accountname);
}

void OwnerNetController::onAccountAdded(const QString &accountname, bool isIam)
{
    qDebug() << "new Account:" << accountname << "is iam:" << isIam;
    if (!isIam)
        return;

    // 新建的域账户需要同步普通账户的设置好的连接的网络信息
    QMap<QString, QString> network;
    QStringList accounts = m_accountManager->primaryAccount();
    for (const QString &accountName : accounts) {
        network = m_networkConfig->network(accountName);
        if (!network.isEmpty())
            break;
    }

    for (auto it = network.begin(); it != network.end(); it++) {
        m_networkConfig->saveNetwork(accountname, it.value(), it.key());
    }

    // 存在这样一种情况，在新建域账户后，PAM会先发送认证请求的信息，此时显示的账户名为...,这个时候需要
    // 更新账户信息，然后再发送请求认证的信号
    m_server->updateIamAuthen(accountname);
}

void OwnerNetController::onAccountRemoved(const QString &accountname)
{
    m_networkConfig->removeNetwork(accountname);
}

void OwnerNetController::onRequestAuthen(const QVariantMap &authenInfo)
{
    qDebug() << "request authen";
    m_networkHandler->disconnectNetwork();
    requestAuthen(authenInfo);
}
