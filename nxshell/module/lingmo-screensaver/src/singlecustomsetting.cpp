// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "singlecustomsetting.h"
#include "customconfig.h"
#include "commandlinehelper.h"

#include <QDir>
#include <QFile>
#include <QLocalServer>
#include <QLocalSocket>
#include <QStandardPaths>
#include <QCommandLineOption>
#include <QCoreApplication>

SingleCustomSetting::SingleCustomSetting(QObject *parent)
    : QObject(parent),
      m_localServer(new QLocalServer)
{
    userServerName();
    initConnect();
}

SingleCustomSetting::~SingleCustomSetting()
{
}

bool SingleCustomSetting::isSingleConfig()
{
    QLocalSocket localSocket;
    localSocket.connectToServer(m_userKey);

    // if connect success, another instance is running.
    if (localSocket.waitForConnected(1000))
        return false;

    m_localServer->removeServer(m_userKey);
    bool f = m_localServer->listen(m_userKey);
    return f;
}

bool SingleCustomSetting::startCustomConfig()
{
    QCommandLineOption optionConfig({ "c", "config" }, "Start config dialog of screen saver.", "screensaer-name", "");
    const QString configName = CommandLineHelper::instance()->getOptionValue(optionConfig);
    if (configName.isEmpty())
        return false;

    if (this->isSingleConfig()) {
        if (!m_conf)
            m_conf = new CustomConfig();
        return m_conf->startCustomConfig(configName);
    } else {
        this->handleNewClient();
        return false;
    }
}

QLocalSocket *SingleCustomSetting::getNewClientConnect(const QByteArray &message)
{
    QLocalSocket *localSocket = new QLocalSocket;
    localSocket->connectToServer(m_userKey);
    if (localSocket->waitForConnected(1000)) {
        if (localSocket->state() == QLocalSocket::ConnectedState) {
            if (localSocket->isValid()) {
                localSocket->write(message);
                localSocket->flush();
            }
        }
    } else {
        qDebug() << localSocket->errorString();
    }

    return localSocket;
}

void SingleCustomSetting::userServerName()
{
    m_userKey = QString("%1/custom_config").arg(QStandardPaths::writableLocation(QStandardPaths::RuntimeLocation));
    if (m_userKey.isEmpty()) {
        m_userKey = QString("%1/custom_config").arg(QStandardPaths::writableLocation(QStandardPaths::TempLocation));
    }
}

void SingleCustomSetting::handleNewClient()
{
    QByteArray data { nullptr };
    bool isSetGetMonitorFiles = false;

    const auto &args = qApp->arguments();
    qDebug() << "new client send data: " << args;
    for (const QString &arg : args) {

        if (!arg.startsWith("-") && QFile::exists(arg)) {
            data.append(QDir(arg).absolutePath().toLocal8Bit().toBase64());
        } else {
            data.append(arg.toLocal8Bit().toBase64());
        }

        data.append(' ');
    }

    if (!data.isEmpty())
        data.chop(1);

    QLocalSocket *socket = SingleCustomSetting::getNewClientConnect(data);
    if (isSetGetMonitorFiles && socket->error() == QLocalSocket::UnknownSocketError) {
        socket->waitForReadyRead();

        for (const QByteArray &i : socket->readAll().split(' '))
            qDebug() << QString::fromLocal8Bit(QByteArray::fromBase64(i));
    }
}

void SingleCustomSetting::initConnect()
{
    connect(m_localServer, &QLocalServer::newConnection, this, &SingleCustomSetting::handleConnection);
}

void SingleCustomSetting::handleConnection()
{
    qDebug() << "new connection is coming";
    QLocalSocket *nextPendingConnection = m_localServer->nextPendingConnection();
    connect(nextPendingConnection, SIGNAL(readyRead()), this, SLOT(readData()));
}

void SingleCustomSetting::readData()
{
    QLocalSocket *socket = qobject_cast<QLocalSocket *>(sender());

    if (!socket)
        return;

    QStringList arguments;
    for (const QByteArray &arg_base64 : socket->read(2048).split(' ')) {
        const QByteArray &arg = QByteArray::fromBase64(arg_base64.simplified());
        if (arg.isEmpty())
            continue;
        arguments << QString::fromLocal8Bit(arg);
    }

    CommandLineHelper::instance()->process(arguments);

    qDebug() << "m_userKey: " << m_userKey;
    qDebug() << "arguments: " << arguments;
    QCommandLineOption optionConfig({ "c", "config" }, "Start config dialog of screen saver.", "screensaer-name", "");
    if (CommandLineHelper::instance()->isSet(optionConfig) && m_conf) {
        QString configName = CommandLineHelper::instance()->getOptionValue(optionConfig);
        if (!configName.isEmpty())
            m_conf->startCustomConfig(configName);
    }
}

void SingleCustomSetting::closeServer()
{
    if (m_localServer) {
        m_localServer->removeServer(m_localServer->serverName());
        m_localServer->close();
        delete m_localServer;
        m_localServer = nullptr;
    }
}
