// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SESSIONWORKER_H
#define SESSIONWORKER_H

#include "session/asioservice.h"
#include "session/protoserver.h"
#include "session/protoclient.h"
#include "sessionproto.h"

#include <string>
#include <unordered_map>
#include <memory>
#include <vector>


class SessionWorker : public SessionCallInterface {
public:
    explicit SessionWorker();
    ~SessionWorker();

    void onReceivedMessage(const proto::OriginMessage &request, proto::OriginMessage *response) override;

    bool onStateChanged(int state, std::string &msg) override;

    void setExtMessageHandler(ExtenMessageHandler cb);

    void setCallbacks(std::shared_ptr<SessionCallback> cb);

    void stop();
    bool startListen(int port);

    bool netTouch(const std::string &address, int port);
    void disconnectRemote();

    std::string sendRequest(const std::string &target, const proto::OriginMessage &request);
    bool sendAsyncRequest(const std::string &target, const proto::OriginMessage &request);

    void updatePincode(const std::string &code);
    void updateLogin(const std::string &ip, bool logined);
    bool isClientLogin(const std::string &ip);

    // 提供处理远程断开和拒绝连接的方法
    void handleRemoteDisconnected(const std::string &remote);
    void handleRejectConnection();

protected:
    void onTransData(const std::string &endpoint, const std::vector<std::string> &nameList);

    void onTransCount(const std::string &names, uint64_t size);

    void onCancelJob(const std::string &jobid, const std::string &reason);

    void onTransResult(const std::string &jobid, const std::string &result);

private:
    bool listen(int port);
    bool connect(const std::string &address, int port);

    template<typename T>
    bool doAsyncRequest(T *endpoint, const std::string &target, const proto::OriginMessage &request);

    std::shared_ptr<AsioService> _asioService;
    // rpc service and client
    std::shared_ptr<ProtoServer> _server { nullptr };
    std::shared_ptr<ProtoClient> _client { nullptr };

    // callback interface
    std::shared_ptr<SessionCallback> _callback { nullptr };

    ExtenMessageHandler _extMsghandler { nullptr };

    std::string _savedPin = "";
    std::string _accessToken = "";
    std::string _connectedAddress = "";

    // mark the connection need to retry after disconneted.
    bool _tryConnect { false };

    // <ip, login>
    std::unordered_map<std::string, bool> _login_hosts;
};

#endif // SESSIONWORKER_H
