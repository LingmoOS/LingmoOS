// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sharecooperationservicemanager.h"
#include "sharecooperationservice.h"
#include "service/comshare.h"
#include <utils/config.h>
#include <utils/cooconfig.h>

#include <QFile>
#include <QTimer>

#include <utils/utils.h>

ShareCooperationServiceManager::ShareCooperationServiceManager(QObject *parent) : QObject(parent)
{
    _client.reset(new ShareCooperationService);
    _client->setBarrierType(BarrierType::Client);
    _server.reset(new ShareCooperationService);
    _server->setBarrierType(BarrierType::Server);
    connect(this, &ShareCooperationServiceManager::startShareServer
            , this, &ShareCooperationServiceManager::handleStartShareSever, Qt::QueuedConnection);
    connect(this, &ShareCooperationServiceManager::stopShareServer
            , this, &ShareCooperationServiceManager::handleStopShareSever, Qt::QueuedConnection);
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
    emit stopShareServer();
}

bool ShareCooperationServiceManager::startServer(const QString &msg)
{
    emit startShareServer(msg);
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

