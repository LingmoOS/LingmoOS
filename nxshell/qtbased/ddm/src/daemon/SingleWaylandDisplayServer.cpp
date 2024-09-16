// Copyright (C) 2023 Dingyuan Zhang <lxz@mkacg.com>.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "SingleWaylandDisplayServer.h"
#include "Messages.h"
#include "SocketServer.h"
#include "Constants.h"
#include "SocketWriter.h"
#include "Utils.h"
#include "Display.h"

#include <QStandardPaths>
#include <QChar>
#include <QLocalSocket>
#include <QLocalServer>
#include <QDataStream>
#include <QTimer>
#include <QProcessEnvironment>

#include <fcntl.h>
#include <sys/socket.h>

using namespace DDM;

SingleWaylandDisplayServer::SingleWaylandDisplayServer(SocketServer *socketServer, Display *parent)
    : DDM::WaylandDisplayServer(parent)
    , m_socketServer(socketServer)
    , m_helper(new QProcess(this))
{
    QProcess *m_seatd = new QProcess(this);
    m_seatd->setProgram("seatd");
    m_seatd->setArguments({"-u", "dde", "-g", "dde", "-l", "debug"});
    m_seatd->setProcessEnvironment([] {
        auto env = QProcessEnvironment::systemEnvironment();
        env.insert("SEATD_VTBOUND", "0");
        return env;
    }());
    connect(m_seatd, &QProcess::readyReadStandardOutput, this, [m_seatd] {
        qInfo() << m_seatd->readAllStandardOutput();
    });
    connect(m_seatd, &QProcess::readyReadStandardError, this, [m_seatd] {
        qWarning() << m_seatd->readAllStandardError();
    });

    connect(m_helper, &QProcess::readyReadStandardOutput, this, [this] {
        qInfo() << m_helper->readAllStandardOutput();
    });
    connect(m_helper, &QProcess::readyReadStandardError, this, [this] {
        qWarning() << m_helper->readAllStandardError();
    });

    m_seatd->start();

    QString socketName = QStringLiteral("treeland-helper-%1").arg(generateName(6));

    // log message
    qDebug() << "Socket server started.";

    connect(m_socketServer, &SocketServer::connected, this, [this, parent](QLocalSocket *socket) {
        m_greeterSockets << socket;
    });
    connect(m_socketServer, &SocketServer::disconnected, this, [this](QLocalSocket *socket) {
        m_greeterSockets.removeOne(socket);
    });

    // TODO: use PAM auth again
    connect(m_socketServer, &SocketServer::requestActivateUser, this, [this]([[maybe_unused]] QLocalSocket *socket, const QString &user){
        activateUser(user);
    });
}

void SingleWaylandDisplayServer::activateUser(const QString &user) {
    for (auto greeter : m_greeterSockets) {
        if (user == "dde") {
            SocketWriter(greeter) << quint32(DaemonMessages::SwitchToGreeter);
        }

        SocketWriter(greeter) << quint32(DaemonMessages::UserActivateMessage) << user;
    }
}

QString SingleWaylandDisplayServer::getUserWaylandSocket(const QString &user) const {
    return m_waylandSockets.value(user);
}

void SingleWaylandDisplayServer::onLoginFailed(const QString &user) {
    for (auto greeter : m_greeterSockets) {
        SocketWriter(greeter) << quint32(DaemonMessages::LoginFailed) << user;
    }
}

void SingleWaylandDisplayServer::onLoginSucceeded(const QString &user) {
    for (auto greeter : m_greeterSockets) {
        SocketWriter(greeter) << quint32(DaemonMessages::LoginSucceeded) << user;
    }
}
