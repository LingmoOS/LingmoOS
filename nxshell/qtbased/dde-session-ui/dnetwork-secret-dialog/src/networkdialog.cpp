// SPDX-FileCopyrightText: 2011 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "networkdialog.h"

#include <QApplication>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingReply>
#include <QDebug>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLocalSocket>

#include <unistd.h>

static const QString NetworkDialogApp = "dde-network-dialog"; //网络列表执行文件
static const QString identity = "identity";
static const QMap<QString, void (NetworkDialog::*)(QLocalSocket *, const QByteArray &)> s_FunMap = {
    { "secretsResult", &NetworkDialog::onSecretsResult },
    { "password", &NetworkDialog::onPassword }
};

NetworkDialog::NetworkDialog(QObject *parent)
    : QObject(parent)
    , m_needIdentity(false)
    , m_quitNow(false)
    , m_clinet(new QLocalSocket(this))
{
    connect(m_clinet, SIGNAL(connected()), this, SLOT(connectedHandler()));
    connect(m_clinet, SIGNAL(disconnected()), this, SLOT(disConnectedHandler()));
    connect(m_clinet, SIGNAL(readyRead()), this, SLOT(readyReadHandler()));
}

NetworkDialog::~NetworkDialog()
{
    qDebug() << "Close socket client, and exit.";
    m_clinet->close();
}

/**
 * @brief NetworkDialog::exec
 *        调dde-network-dialog处理密码输入
 *        参数为空或为WPA2企业版需要输入多个项时返回false，用原输入窗处理
 * @param doc json对象
 * @return true 已处理 false 未处理
 */
bool NetworkDialog::exec(const QJsonDocument &doc)
{
    QJsonObject obj = doc.object();
    // 只处理无线网密码
    if (obj.value("connType").toString() != "802-11-wireless") {
        qDebug() << "Only deal with wireless for the connection type, now connType:" << obj.value("connType");
        return false;
    }
    QString connName = obj.value("connId").toString();
    if (!connName.isEmpty()) {
        m_data = "\nrequestSecrets:" + doc.toJson(QJsonDocument::Compact) + "\n";
        QJsonArray array = obj.value("devices").toArray();
        QString device;
        if (!array.isEmpty()) {
            device = array.first().toString();
        }
        QJsonObject propsObj = obj.value("props").toObject();
        QString password;
        int count = 0;
        for (auto secret : obj.value("secrets").toArray()) {
            const QString& key = secret.toString();
            if (key == identity) {
                m_needIdentity = true;
                continue;
            }
            count++;
            if (propsObj.contains(key)) {
                password = propsObj.value(key).toString();
            }
        }
        m_key = connName;
        QJsonObject json;
        json.insert("dev", device);
        json.insert("ssid", m_key);
        json.insert("wait", true);
        if (m_needIdentity) {
            m_identityContent = propsObj.value(identity).toString();
            json.insert(identity, m_identityContent);
        }
        json.insert("password", password);
        QJsonDocument passwordDoc;
        passwordDoc.setObject(json);
        m_data += "\nconnect:" + passwordDoc.toJson(QJsonDocument::Compact) + "\n";
        ConnectToServer();
        qDebug() << "Connect to server :" << m_clinet->serverName() << ", ssid:" << m_key;
        return true;
    }
    return false;
}

void NetworkDialog::onSecretsResult(QLocalSocket *socket, const QByteArray &data)
{
    Q_UNUSED(socket);
    if (data.isEmpty()) {
        exit(1);
        return;
    }

    QFile file;
    if (!file.open(stdout, QFile::WriteOnly)) {
        qWarning() << "Open stdout failed";
        exit(-4);
    }
    file.write(data);
    file.flush();
    file.close();
    exit(0);
}

void NetworkDialog::onPassword(QLocalSocket *socket, const QByteArray &data)
{
    Q_UNUSED(socket);
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject())
        return;
    QJsonObject obj = doc.object();
    QString key = obj.value("key").toString();
    QString passwd = obj.value("password").toString();
    QString identityContent;
    bool input = obj.value("input").toBool();
    if (m_key != key)
        return;
    if (!input) {
        exit(1);
        return;
    }
    QJsonObject resultJsonObj;
    QJsonArray secretsJsonArray;
    if (m_needIdentity) {
        // 如果从networkcore获取的用户身份信息为空,则使用调用者传进来的数据.(用于兼容dde-network-core低版本情况)
        identityContent = obj.value(identity).toString().isEmpty() ? m_identityContent : obj.value(identity).toString();
        secretsJsonArray.append(identityContent);
    }
    secretsJsonArray.append(passwd);
    resultJsonObj.insert("secrets", secretsJsonArray);
    QFile file;
    if (!file.open(stdout, QFile::WriteOnly)) {
        qDebug() << "open STDOUT failed";
        exit(-4);
    }
    file.write(QJsonDocument(resultJsonObj).toJson());
    file.flush();
    file.close();
    exit(0);
}

void NetworkDialog::connectedHandler()
{
    if (!m_clinet->isOpen() || m_data.isEmpty())
        return;

    m_clinet->write(m_data);
    m_clinet->flush();
}

void NetworkDialog::disConnectedHandler()
{
    qDebug() << "Received disconnect event from the socket and exit it.";
    exit(0);
}

bool NetworkDialog::ConnectToServer()
{
    // 区分登录、锁屏和任务栏
    // 当sessionManager服务没起来时是在登录界面，锁屏和任务栏通过locked属性来判断
    QDBusMessage dbusMessage = QDBusMessage::createMethodCall("org.deepin.dde.SessionManager1", "/org/deepin/dde/SessionManager1", "org.freedesktop.DBus.Properties", "Get");
    dbusMessage << "org.deepin.dde.SessionManager1" << "Locked";
    QDBusPendingReply<QVariant> prop = QDBusConnection::sessionBus().call(dbusMessage, QDBus::Block, 1000);

    QString serverName = NetworkDialogApp + QString::number(getuid());
    if (prop.value().toBool()) {
        serverName += "lock";
    } else {
        serverName += "dock";
    }
    m_clinet->connectToServer(serverName);
    m_clinet->waitForConnected();
    QLocalSocket::LocalSocketState state = m_clinet->state();
    return state == QLocalSocket::ConnectedState;
}

void NetworkDialog::exit(int returnCode)
{
    if (m_quitNow)
        return;

    m_quitNow = true;
    qApp->exit(returnCode);
}

void NetworkDialog::readyReadHandler()
{
    QLocalSocket *socket = static_cast<QLocalSocket *>(sender());
    if (!socket)
        return;

    QByteArray allData = socket->readAll();
    allData = m_lastData + allData;
    QList<QByteArray> dataArray = allData.split('\n');
    m_lastData = dataArray.last();
    for (const QByteArray &data : dataArray) {
        if (m_quitNow)
            return;

        int keyIndex = data.indexOf(':');
        if (keyIndex == -1)
            continue;

        QString key = data.left(keyIndex);
        QByteArray value = data.mid(keyIndex + 1);
        if (s_FunMap.contains(key)) {
            (this->*s_FunMap.value(key))(socket, value);
        }
    }
}
