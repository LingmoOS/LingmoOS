// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "singleinstance.h"
#include <QtNetwork/qlocalserver.h>
#include <QLocalServer>
#include <QLocalSocket>
#include <QStandardPaths>
#include <QApplication>
#include <QDir>

SingleInstance::SingleInstance(const QString &name, QObject *parent)
    : QObject(parent)
    , m_localServer(new QLocalServer)
{
    m_userKey = QString("%1/%2").arg(QStandardPaths::writableLocation(QStandardPaths::RuntimeLocation)).arg(name);
    if (m_userKey.isEmpty()) {
        m_userKey = QString("%1/%2").arg(QStandardPaths::writableLocation(QStandardPaths::TempLocation)).arg(name);
    }

    initConnect();
}

SingleInstance::~SingleInstance()
{
    closeServer();
}

bool SingleInstance::isSingle()
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

void SingleInstance::closeServer()
{
    if (m_localServer) {
        m_localServer->removeServer(m_localServer->serverName());
        m_localServer->close();
        delete m_localServer;
        m_localServer = nullptr;
    }
}

void SingleInstance::sendNewClient()
{
    QByteArray data { nullptr };
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

    QLocalSocket *socket = getNewClientConnect(data);
    socket->deleteLater();
}

QLocalSocket *SingleInstance::getNewClientConnect(const QByteArray &message)
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

void SingleInstance::handleConnection()
{
    qDebug() << "new connection is coming";
    QLocalSocket *nextPendingConnection = m_localServer->nextPendingConnection();
    connect(nextPendingConnection, SIGNAL(readyRead()), this, SLOT(readData()));
}

void SingleInstance::readData()
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

    emit handArguments(arguments);
}

void SingleInstance::initConnect()
{
    connect(m_localServer, &QLocalServer::newConnection, this, &SingleInstance::handleConnection);
}
