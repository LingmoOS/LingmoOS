// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PROTOENDPOINT_H
#define PROTOENDPOINT_H

#include "session.h"
#include "server/asio/timer.h"

#include <atomic>
#include <iostream>

// hearbeat timeout
#define HEARTBEAT_INTERVAL 2

using CppServer::Asio::Timer;
using RpcHandler = std::function<void(int32_t type, const std::string &response)>;

class ProtoEndpoint: public FBE::proto::FinalClient
{
public:

    void setCallbacks(std::shared_ptr<SessionCallInterface> callbacks);

    void sendDisRequest();
    proto::OriginMessage syncRequest(const std::string &target, const proto::OriginMessage &msg);

    // async call request and with 3s timeout result callback
    void asyncRequestWithHandler(const std::string &target, const proto::OriginMessage &request, RpcHandler resultHandler);

    virtual bool hasConnected(const std::string &ip) { return false; }

private:
    void asyncRequest(const std::string &target, const proto::OriginMessage &msg);
    std::string getResponse(int32_t type);

protected:
    std::shared_ptr<SessionCallInterface> _callbacks { nullptr };

    //mask self request, default false for remote
    std::atomic<bool> _self_request { false };

    //current active request target
    std::string _active_traget = { "" };

private:
    std::mutex _lock;
    // type, future
    std::unordered_map<int32_t, std::future<proto::OriginMessage>> _responses_by_type;
};

#endif // PROTOENDPOINT_H
