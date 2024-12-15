// SPDX-FileCopyrightText: 2023-2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sharecooperationservicemanager.h"
#include "sharecooperationservice.h"

#include <QFile>
#include <QTimer>
#include <QStandardPaths>

ShareCooperationServiceManager::ShareCooperationServiceManager(QObject *parent)
    : QObject(parent)
{
    _client.reset(new ShareCooperationService);
    _client->setBarrierType(BarrierType::Client);
    _client->setBarrierProfile(barrierProfile());
    _server.reset(new ShareCooperationService);
    _server->setBarrierType(BarrierType::Server);
    _server->setBarrierProfile(barrierProfile());
    connect(this, &ShareCooperationServiceManager::startShareServer, this, &ShareCooperationServiceManager::handleStartShareSever, Qt::QueuedConnection);
    connect(this, &ShareCooperationServiceManager::stopShareServer, this, &ShareCooperationServiceManager::handleStopShareSever, Qt::QueuedConnection);
}

ShareCooperationServiceManager::~ShareCooperationServiceManager()
{
    stop();
}

ShareCooperationServiceManager *ShareCooperationServiceManager::instance()
{
    static ShareCooperationServiceManager in;
    return &in;
}

QSharedPointer<ShareCooperationService> ShareCooperationServiceManager::client()
{
    return _client;
}

QSharedPointer<ShareCooperationService> ShareCooperationServiceManager::server()
{
    return _server;
}

void ShareCooperationServiceManager::stop()
{
    _client->stopBarrier();
    _client->setClientTargetIp(""); // reset client by serverIp
    emit stopShareServer();
}

bool ShareCooperationServiceManager::startServer(const QString &msg)
{
    emit startShareServer(msg);
    return true;
}

bool ShareCooperationServiceManager::stopServer()
{
    emit stopShareServer();
    return true;
}

void ShareCooperationServiceManager::handleStartShareSever(const QString msg)
{
    if (_server.isNull())
        return;
    bool ok = _server->restartBarrier();
    emit startServerResult(ok, msg);
}

void ShareCooperationServiceManager::handleStopShareSever()
{
    if (_server.isNull())
        return;
    _server->stopBarrier();
}

QString ShareCooperationServiceManager::barrierProfile()
{
    QString profileDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    return profileDir;
}
