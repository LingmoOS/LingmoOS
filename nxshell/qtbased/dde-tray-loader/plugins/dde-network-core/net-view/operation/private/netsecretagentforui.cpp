// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "netsecretagentforui.h"

#include "networkconst.h"

#include <QApplication>
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLocalServer>
#include <QLocalSocket>
#include <QTimer>

#include <unistd.h>

namespace dde {
namespace network {

static QMap<QString, void (NetSecretAgentForUI::*)(QLocalSocket *, const QByteArray &)> s_FunMap = {
    { "requestSecrets", &NetSecretAgentForUI::requestSecrets },
};

NetSecretAgentForUI::NetSecretAgentForUI(PasswordCallbackFunc fun, const QString &serverKey, QObject *parent)
    : QObject(parent)
    , NetSecretAgentInterface(fun)
    , m_server(nullptr)
{
    setServerName("dde-network-dialog" + QString::number(getuid()) + serverKey);
}

bool NetSecretAgentForUI::hasTask()
{
    return !m_connectSsid.isEmpty();
}

void NetSecretAgentForUI::inputPassword(const QString &key, const QVariantMap &password, bool input)
{
    sendSecretsResult(key, password, input);
}

NetSecretAgentForUI::~NetSecretAgentForUI() = default;

void NetSecretAgentForUI::setServerName(const QString &name)
{
    if (m_server)
        return;

    m_serverName = name;
    m_server = new QLocalServer(this);
    connect(m_server, &QLocalServer::newConnection, this, &NetSecretAgentForUI::newConnectionHandler);
    m_server->setSocketOptions(QLocalServer::WorldAccessOption);

    m_server->listen(m_serverName);
}

void NetSecretAgentForUI::newConnectionHandler()
{
    QLocalSocket *socket = m_server->nextPendingConnection();
    connect(socket, &QLocalSocket::readyRead, this, &NetSecretAgentForUI::readyReadHandler);
    connect(socket, &QLocalSocket::disconnected, this, &NetSecretAgentForUI::disconnectedHandler);
    QTimer::singleShot(GET_SECRETS_TIMEOUT, socket, &QLocalSocket::disconnectFromServer);
    m_clients.append(socket);
}

void NetSecretAgentForUI::disconnectedHandler()
{
    auto *socket = dynamic_cast<QLocalSocket *>(sender());
    if (socket) {
        if (!m_connectSsid.isEmpty())
            cancelRequestPassword(m_connectDev, m_connectSsid);
        m_clients.removeAll(socket);
        socket->deleteLater();
        m_connectSsid.clear();
    }
}

void NetSecretAgentForUI::sendSecretsResult(const QString &key, const QVariantMap &password, bool input)
{
    Q_UNUSED(key)
    m_connectSsid.clear();
    QByteArray data;
    if (input) {
        QStringList secrets;
        for (auto &&secret : m_secrets) {
            secrets << password.value(secret).toString();
        }
        QJsonObject json;
        json.insert("secrets", QJsonArray::fromStringList(secrets));
        QJsonDocument doc;
        doc.setObject(json);
        data = doc.toJson(QJsonDocument::Compact);
    }
    for (auto &m_client : m_clients) {
        m_client->write("\nsecretsResult:" + data + "\n");
    }
}

void NetSecretAgentForUI::readyReadHandler()
{
    auto *socket = dynamic_cast<QLocalSocket *>(sender());
    if (!socket)
        return;

    QByteArray allData = socket->readAll();
    allData = m_lastData + allData;
    QList<QByteArray> dataArray = allData.split('\n');
    m_lastData = dataArray.last();
    for (const QByteArray &data : dataArray) {
        int keyIndex = data.indexOf(':');
        if (keyIndex != -1) {
            QString key = data.left(keyIndex);
            QByteArray value = data.mid(keyIndex + 1);
            if (s_FunMap.contains(key)) {
                (this->*s_FunMap.value(key))(socket, value);
            }
        }
    }
}

void NetSecretAgentForUI::requestSecrets(QLocalSocket *socket, const QByteArray &data)
{
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isObject()) {
        QJsonObject obj = doc.object();

        QVariantMap param;
        QStringList secrets;
        QMap<QString, QString> prop;
        QString dev;
        QJsonArray devices = obj.value("devices").toArray();
        if (!devices.isEmpty()) {
            dev = devices.first().toString();
        }
        const QJsonArray &jsonSecrets = obj.value("secrets").toArray();
        for (auto &&s : jsonSecrets) {
            secrets << s.toString();
        }
        const QJsonObject &jsonProps = obj.value("props").toObject();
        for (auto p = jsonProps.constBegin(); p != jsonProps.constEnd(); ++p) {
            prop.insert(p.key(), p.value().toString());
        }
        m_connectDev = dev;
        m_connectSsid = obj.value("connId").toString();
        m_secrets = secrets;
        param.insert("secrets", secrets);
        if (!prop.isEmpty()) {
            param.insert("prop", QVariant::fromValue(prop));
        }
        requestPassword(dev, m_connectSsid, param);
    }
    socket->write("\nreceive:" + data + "\n");
}

} // namespace network
} // namespace dde
