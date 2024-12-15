// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SERVICEMANAGER_H
#define SERVICEMANAGER_H

#include <QObject>
#include <QSharedPointer>
#include <QTimer>

#include "co/co.h"
#include "co/json.h"

class RemoteServiceSender;
class DiscoveryJob;
class HandleIpcService;
class HandleRpcService;
class HandleSendResultService;
class ServiceManager : public QObject
{
    Q_OBJECT
public:
    explicit ServiceManager(QObject *parent = nullptr);
    ~ServiceManager();

    void startRemoteServer();

public slots:
    void handleAppQuit();

private:
    void localIPCStart();
    fastring genPeerInfo();
    void asyncDiscovery();
private:
    HandleIpcService *_ipcService { nullptr };
    HandleRpcService *_rpcService { nullptr };
    QSharedPointer<HandleSendResultService> _logic;
    QTimer _userTimer;
};

#endif // SERVICEMANAGER_H
