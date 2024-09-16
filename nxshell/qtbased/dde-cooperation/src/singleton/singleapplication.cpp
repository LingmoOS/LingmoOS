// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "singleapplication.h"

#include <QLocalServer>
#include <QLocalSocket>
#include <QStandardPaths>
#include <QFile>
#include <QDir>
#include <QWidget>

using namespace deepin_cross;

SingleApplication::SingleApplication(int &argc, char **argv, int)
    : CrossApplication(argc, argv), localServer(new QLocalServer(this))
{
    setOrganizationName("deepin");
    setAttribute(Qt::AA_UseHighDpiPixmaps);
    initConnect();
}

SingleApplication::~SingleApplication()
{
    closeServer();
}

void SingleApplication::initConnect()
{
    connect(localServer, &QLocalServer::newConnection, this, &SingleApplication::handleConnection);
}

void SingleApplication::handleConnection()
{
    qDebug() << "new connection is coming";
    auto windowList = qApp->topLevelWidgets();
    for (auto w : windowList) {
        if (w->objectName() == "MainWindow") {
            w->raise();
            w->activateWindow();
            break;
        }
    }

    QLocalSocket *nextPendingConnection = localServer->nextPendingConnection();
    connect(nextPendingConnection, SIGNAL(readyRead()), this, SLOT(readData()));
}

bool SingleApplication::sendMessage(const QString &key, const QByteArray &message)
{
    QLocalSocket *localSocket = new QLocalSocket;
    localSocket->connectToServer(userServerName(key));
    if (localSocket->waitForConnected(1000)) {
        if (localSocket->state() == QLocalSocket::ConnectedState) {
            if (localSocket->isValid()) {
                localSocket->write(message);
                localSocket->flush();
                return true;
            }
        }
    } else {
        qDebug() << localSocket->errorString();
    }

    return false;
}

bool SingleApplication::checkProcess(const QString &key)
{
    QString userKey = userServerName(key);

    QLocalSocket localSocket;
    localSocket.connectToServer(userKey);

    // if connect success, another instance is running.
    bool result = localSocket.waitForConnected(1000);
#ifdef __linux__
    if (!result) {
        // check the /tmp socket file for linux
        userKey = QString("%1/%2").arg(QStandardPaths::writableLocation(QStandardPaths::TempLocation), key);
        result = QFile::exists(userKey);
    }
#endif
    qWarning() << "checkProcess " << userKey << result;

    return result;
}

bool SingleApplication::setSingleInstance(const QString &key)
{
    if (checkProcess(key))
        return false;

    QString userKey = userServerName(key);
    localServer->removeServer(userKey);

    bool f = localServer->listen(userKey);

    return f;
}

void SingleApplication::readData()
{
    QLocalSocket *socket = qobject_cast<QLocalSocket *>(sender());

    if (!socket)
        return;

    QStringList arguments;
    arguments << this->arguments().first(); // append self app name at first in order to parse commandline.
    for (const QByteArray &arg_base64 : socket->readAll().split(' ')) {
        const QByteArray &arg = QByteArray::fromBase64(arg_base64.simplified());

        if (arg.isEmpty())
            continue;

        arguments << QString::fromLocal8Bit(arg);
    }

    emit SingleApplication::onArrivedCommands(arguments);
}

void SingleApplication::closeServer()
{
    if (localServer) {
        localServer->removeServer(localServer->serverName());
        localServer->close();
        delete localServer;
        localServer = nullptr;
    }
}

void SingleApplication::helpActionTriggered()
{
#ifdef linux
    DApplication::handleHelpAction();
#endif
}

void SingleApplication::onDeliverMessage(const QString &app, const QStringList &msg)
{
    QByteArray data { nullptr };
    for (const QString &arg : msg) {
        data.append(arg.toLocal8Bit().toBase64());
        data.append(' ');
    }

    if (!data.isEmpty()) {
        data.chop(1);
        sendMessage(app, data);
    }
}

QString SingleApplication::userServerName(const QString &key)
{
    QString userKey = QString("%1/%2").arg(QStandardPaths::writableLocation(QStandardPaths::RuntimeLocation), key);
    if (userKey.isEmpty()) {
        userKey = QString("%1/%2").arg(QStandardPaths::writableLocation(QStandardPaths::TempLocation), key);
    }
    return userKey;
}
