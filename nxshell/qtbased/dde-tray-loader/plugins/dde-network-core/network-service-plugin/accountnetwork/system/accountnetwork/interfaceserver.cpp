// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "interfaceserver.h"
#include "accountmanager.h"
#include "networkconfig.h"

#include <DDBusSender>

#include <QLocalServer>
#include <QLocalSocket>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDBusInterface>
#include <QDBusPendingCall>
#include <QDBusPendingReply>
#include <QDBusObjectPath>
#include <QMetaType>
#include <QDBusArgument>
#include <QDBusMetaType>
#include <QFile>
#include <QProcess>

using namespace accountnetwork::systemservice;

InterfaceServer::InterfaceServer(AccountManager *accountManager, NetworkConfig *conf, QObject *parent)
    : QObject (parent)
    , m_localServer(new QLocalServer(this))
    , m_accountManager(accountManager)
    , m_networkConfig(conf)
    , m_needAuthen(false)
{
    m_localServer->removeServer("PAMServer");
    m_localServer->setSocketOptions(QLocalServer::WorldAccessOption);
    if (m_localServer->listen("PAMServer")) {
        qInfo() << "start PAM Server success.";
        connect(m_localServer, &QLocalServer::newConnection, this, &InterfaceServer::newConnectionHandler);
    }
}

InterfaceServer::~InterfaceServer()
{
    m_localServer->removeServer("PAMServer");
}

QVariantMap InterfaceServer::authenInfo(const QString &accountname) const
{
    if (m_authenInfo.contains(accountname))
        return m_authenInfo[accountname];

    return QVariantMap();
}

void InterfaceServer::updateIamAuthen(const QString &accountname)
{
    // 如果认证账户中包含当前的账户的信息，则不做任何处理
    if (!m_needAuthen)
        return;

    m_needAuthen = false;
    // 开始请求认证
    qDebug() << "iam account start authen...";
    m_authenInfo[accountname] = m_authen;
    emit requestAuthen(m_authen);
    m_authen.clear();
}

void InterfaceServer::newConnectionHandler()
{
    QLocalSocket *socket = m_localServer->nextPendingConnection();
    connect(socket, &QLocalSocket::readyRead, this, &InterfaceServer::readyReadHandler);
    connect(socket, &QLocalSocket::disconnected, this, &InterfaceServer::disconnectedHandler);
    QTimer::singleShot(12000, socket, &QLocalSocket::disconnectFromServer);
    m_clients.append(socket);
}

void InterfaceServer::readyReadHandler()
{
    QLocalSocket *socket = qobject_cast<QLocalSocket *>(sender());
    if (!socket)
        return;

    do {
        if (m_accountManager->account().isEmpty()) {
            QString message = "current account is empty";
            qWarning() << message;
            socket->write(message.toStdString().c_str());
            break;
        }

        QByteArray allData = socket->readAll();
        qDebug() << "Recieve data from client: " << allData;
        QJsonParseError error;
        QJsonDocument json = QJsonDocument::fromJson(allData, &error);
        if (!json.isObject()) {
            QString errMsg = QString("json is not valid, error message:%1, content:%2").arg(error.errorString()).arg(QString(allData));
            qWarning() << errMsg;
            socket->write(errMsg.toStdString().c_str());
            break;
        }

        QVariantMap authenInfo;
        QJsonObject jsonObject = json.object();
        if (!jsonObject.contains("type")) {
            QString errMsg = "message must contain type";
            socket->write(errMsg.toStdString().c_str());
            break;
        }
        QString type = jsonObject.value("type").toString();
        if (type == "authen") {
            m_authen.clear();
            m_needAuthen = false;
            if (jsonObject.contains("identity"))
                authenInfo.insert("identity", jsonObject.value("identity").toString());
            if (jsonObject.contains("password"))
                authenInfo.insert("password", jsonObject.value("password").toString());

            QString account = m_accountManager->account();
            if (m_accountManager->accountExist(account)) {
                qInfo() << "account exist, start authen, current acocunt:" << account;
                m_authenInfo[m_accountManager->account()] = authenInfo;
                emit requestAuthen(authenInfo);
            } else {
                // 如果当前账户信息为空，则先记录下认证信息，等待账户返回后，再请求连接
                qWarning() << "account can't exist, wait for it added, account name:" << account;
                m_authen = authenInfo;
                m_needAuthen = true;
            }
            socket->write("success");
        } else if (type == "disconnect") {
            // 这个消息由客户端发送，用于手动断开连接，此时主要用于记录手动断开连接的流程
            if (!jsonObject.contains("interface") || !jsonObject.contains("id")) {
                QString message = "does not have interface and id";
                qWarning() << message;
                socket->write(message.toStdString().c_str());
                break;
            }
            QString id = jsonObject.value("id").toString();
            QString deviceInterface = jsonObject.value("interface").toString();
            qInfo() << deviceInterface << " disconnect network" << id;
            m_networkConfig->removeNetwork(m_accountManager->account(), id, deviceInterface);
            socket->write("success");
        }
    } while (0);

    socket->disconnectFromServer();
}

void InterfaceServer::disconnectedHandler()
{
    QLocalSocket *socket = qobject_cast<QLocalSocket *>(sender());
    if (!socket)
        return;

    m_clients.removeAll(socket);
    socket->deleteLater();
}
