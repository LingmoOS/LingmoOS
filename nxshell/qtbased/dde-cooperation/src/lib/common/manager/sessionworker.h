// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SESSIONWORKER_H
#define SESSIONWORKER_H

#include "session/asioservice.h"
#include "session/protoserver.h"
#include "session/protoclient.h"

#include "sessionproto.h"

#include <QObject>
#include <QMap>

class SessionWorker : public QObject, public SessionCallInterface
{
    Q_OBJECT
public:
    explicit SessionWorker(QObject *parent = nullptr);
    ~SessionWorker();

    void onReceivedMessage(const proto::OriginMessage &request, proto::OriginMessage *response) override;

    bool onStateChanged(int state, std::string &msg) override;

    void setExtMessageHandler(ExtenMessageHandler cb);

    void stop();
    bool startListen(int port);

    bool netTouch(QString &address, int port);
    void disconnectRemote();

    QString sendRequest(const QString &target, const proto::OriginMessage &request);
    bool sendAsyncRequest(const QString &target, const proto::OriginMessage &request);

    void updatePincode(QString code);
    void updateLogin(QString ip, bool logined);
    bool isClientLogin(QString &ip);

signals:
    void onTransData(const QString endpoint, const QStringList nameVector);
    void onTransCount(const QString names, quint64 size);
    void onCancelJob(const QString jobid, const QString reason);
    void onConnectChanged(int result, QString reason);

    // local signals which emit from RPC
    void onRemoteDisconnected(const QString &remote);

    void onRejectConnection();

    // rpc async call result
    void onRpcResult(int32_t type, const QString &response);

    // I/O exception
    void onNetException(const QString &remote);
public slots:
    void handleRemoteDisconnected(const QString &remote);

    void handleRejectConnection();

private:
    bool listen(int port);
    bool connect(QString &address, int port);

    template<typename T>
    bool doAsyncRequest(T *endpoint, const std::string &target, const proto::OriginMessage &request);

    std::shared_ptr<AsioService> _asioService;
    // rpc service and client
    std::shared_ptr<ProtoServer> _server { nullptr };
    std::shared_ptr<ProtoClient> _client { nullptr };

    ExtenMessageHandler _extMsghandler { nullptr };

    QString _savedPin = "";
    QString _accessToken = "";
    QString _connectedAddress = "";

    // mark the connection need to retry after disconneted.
    bool _tryConnect { false };

    // <ip, login>
    QMap<QString, bool> _login_hosts;
};

#endif // SESSIONWORKER_H
