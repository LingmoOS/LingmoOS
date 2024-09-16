// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SENDIPCSERVICE_H
#define SENDIPCSERVICE_H

#include <QObject>
#include <QMap>
#include <QTimer>
#include <QSharedPointer>
#include <QThread>

#include "co/json.h"
#include "ipc/proto/chan.h"

class Session;

class SendIpcService : public QObject
{
    Q_OBJECT
public:
    ~SendIpcService();
    static SendIpcService *instance();

signals:

    void startOfflineTimer();
    void stopOfflineTimer();

    void sessionSignal(const QString& signalName, int type, const QString& message);

public slots:
    void handleSaveSession(const QString appName, const QString sessionID, const QString signalName);
    // void handleConnectClosed(const uint16 port);
    void handleRemoveSessionByAppName(const QString appName);
    void handleRemoveSessionBySessionID(const QString sessionID);
    void handleSendToClient(const QString appName, int type, const QString msg);
    void handleSendToAllClient(int type, const QString msg);
    void handleAboutToQuit();
    void handleAddJob(const QString appName, const int jobId);
    void handleRemoveJob(const QString appName, const int jobId);

    //缓存远端离线消息，可能因某次失败而触发，若3秒内有正常则应取消此消息到前端
    void preprocessOfflineStatus(const QString appName, int32 type, const fastring msg);
    void cancelOfflineStatus(const QString appName);

    void handleStartOfflineTimer();
    void handleStopOfflineTimer();

    void handleNodeChanged(bool found, QString info);

private:
    explicit SendIpcService( QObject *parent = nullptr);
    void initConnect();

private:
    QTimer _ping;
    QMap<QString, SendStatus> _offline_status;
    QTimer _cacheTimer;

    // record the frontend session
    QMap<QString, QSharedPointer<Session>> _sessions;
};

#endif // SENDIPCSERVICE_H
