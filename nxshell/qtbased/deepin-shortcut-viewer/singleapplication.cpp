// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "singleapplication.h"
#include "commandlinemanager.h"
#include "view/mainwidget.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QDir>

QString SingleApplication::UserID = "1000";

SingleApplication::SingleApplication(int &argc, char **argv, int)
    : DApplication(argc, argv)
{
    m_localServer = new QLocalServer;
    initConnect();
}

SingleApplication::~SingleApplication()
{
    if (m_localServer) {
        m_localServer->removeServer(m_localServer->serverName());
        m_localServer->close();
    }
}

void SingleApplication::initConnect()
{
    connect(m_localServer, &QLocalServer::newConnection, this, &SingleApplication::handleConnection);
}

void SingleApplication::newClientProcess(const QString &key, const QByteArray &message)
{
    qDebug() << "The deepin-shortcut-viewer is running!";
    QLocalSocket *localSocket = new QLocalSocket;
    localSocket->connectToServer(userServerName(key));
    if (localSocket->waitForConnected(1000)) {
        if (localSocket->state() == QLocalSocket::ConnectedState) {
            if (localSocket->isValid()) {
                qDebug() << "start write";
                localSocket->write(message);
                localSocket->flush();
            }
        }
    } else {
        qDebug() << localSocket->errorString();
    }
    qDebug() << "The deepin-shortcut-viewer is running end!";
}

QString SingleApplication::userServerName(const QString &key)
{
    QString userKey;
    if (userID() == "0") {
        userKey = QString("%1/%2").arg("/tmp", key);
    } else {
        userKey = QString("%1/%2/%3").arg("/var/run/user", userID(), key);
    }
    qDebug() << userKey;
    return userKey;
}

QString SingleApplication::userID()
{
    QProcess userID;
    userID.start("id", QStringList() << "-u");
    userID.waitForFinished();
    QByteArray id = userID.readAll();
    UserID = QString(id).trimmed();
    return UserID;
}

void SingleApplication::processArgs(const QStringList &list)
{
    //Command manager
    CommandLineManager cmdManager;
    cmdManager.process(list);

    QString jsonData = cmdManager.jsonData();
    QPoint pos = cmdManager.pos();

    static MainWidget *w = Q_NULLPTR;

    if (jsonData == "")
        return;

    if (w && cmdManager.enableBypassWindowManagerHint()) {
        w->deleteLater();
        w = Q_NULLPTR;
    }

    if (!w)
        w = new MainWidget();

    w->setJsonData(jsonData);
    pos -= QPoint(w->width() / 2, w->height() / 2);

    if (cmdManager.enableBypassWindowManagerHint())
        w->setWindowFlags(w->windowFlags() | Qt::BypassWindowManagerHint);

    w->show();
    w->move(pos);
    //w->activateWindow();
    w->setFocus();
}

bool SingleApplication::setSingleInstance(const QString &key)
{
    QString userKey = userServerName(key);

    QLocalSocket *localSocket = new QLocalSocket;
    localSocket->connectToServer(userKey);

    // if connect success, another instance is running.
    bool result = localSocket->waitForConnected(1000);
    localSocket->deleteLater();

    if (result)
        return false;

    m_localServer->removeServer(userKey);

    bool f = m_localServer->listen(userKey);

    return f;
}

void SingleApplication::handleConnection()
{
    qDebug() << "new connection is coming";
    QLocalSocket *nextPendingConnection = m_localServer->nextPendingConnection();
    connect(nextPendingConnection, SIGNAL(readyRead()), this, SLOT(readData()));
}

void SingleApplication::readData()
{
    const QByteArray &message = qobject_cast<QLocalSocket *>(sender())->readAll();

    QStringList list;

    for (const QByteArray &data : message.split('\0'))
        list << QString::fromLocal8Bit(data);

    processArgs(list);
}
