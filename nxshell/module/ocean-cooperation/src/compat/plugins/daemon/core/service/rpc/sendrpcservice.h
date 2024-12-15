// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SENDRPCSERVICE_H
#define SENDRPCSERVICE_H

#include "service/rpc/remoteservice.h"

#include <QObject>
#include <QMap>
#include <QTimer>
#include <QReadLocker>
#include <QThread>
#include <QSharedPointer>

#include "co/json.h"

class SendRpcWork : public QObject {
    Q_OBJECT
    friend class SendRpcService;
public:
    ~SendRpcWork();
    void stop() {_stoped = true;}

Q_SIGNALS:
    void sendToRpcResult(const QString appName, const QString msg);

private:
    explicit SendRpcWork( QObject *parent = nullptr);

private slots:
    void handleCreateRpcSender(const QString appName,
                               const QString targetip, quint16 port);
    void handleSetTargetAppName(const QString appName, const QString targetAppName);
    void handleDoSendProtoMsg(const uint32 type, const QString appName,
                              const QString msg, const QByteArray data);

    void handlePing(const QStringList apps);

private:
    QSharedPointer<RemoteServiceSender> createRpcSender(const QString &appName,
                                                        const QString &targetip, uint16_t port);
    QSharedPointer<RemoteServiceSender> rpcSender(const QString &appName);

private:
    // <ip, remote>
    QMap<QString, QSharedPointer<RemoteServiceSender>> _remotes;
    // <appName, ip>
    QMap<QString, QString> _app_ips;
    std::atomic_bool _stoped{false};

    QMap<QString, int> _ping_failed_count;
};

class SendRpcService : public QObject
{
    Q_OBJECT
Q_SIGNALS:
    void startPingTimer();
    void stopPingTimer();
    void sendToRpcResult(const QString appName, const QString msg);

    void workCreateRpcSender(const QString appName,
                             const QString targetip, quint16 port);
    void workSetTargetAppName(const QString appName, const QString targetAppName);
    void workRemovePing(const QString appName);
    void workDoSendProtoMsg(const uint32 type, const QString AppName,
                            const QString msg, const QByteArray data);

    void ping(const QStringList apps);

public slots:
    void handleStartTimer();
    void handleStopTimer();
    void handleTimeOut();
    void handleAboutQuit();

public:
    ~SendRpcService() override;
    static SendRpcService *instance();

    void createRpcSender(const QString &appName,
                         const QString &targetip, quint16 port) {
        emit workCreateRpcSender(appName, targetip, port);
    }


    void setTargetAppName(const QString &appName, const QString &targetAppName) {
        emit workSetTargetAppName(appName, targetAppName);
    }
    void doSendProtoMsg(const uint32 type, const QString &appName,
                        const QString &msg, const QByteArray &data = QByteArray()) {
        emit workDoSendProtoMsg(type, appName, msg, data);
    }

    void removePing(const QString &appName);

    void addPing(const QString &appName);

private:
    explicit SendRpcService( QObject *parent = nullptr);

    void initConnet();

private:
    SendRpcWork _work;
    QThread _thread;
    QReadWriteLock _ping_lock;
    QStringList _ping_appname;
    QTimer _ping_timer;
};

#endif // SENDRPCSERVICE_H
