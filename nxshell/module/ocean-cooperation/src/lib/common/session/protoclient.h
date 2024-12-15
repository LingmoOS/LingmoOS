// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PROTOCLIENT_H
#define PROTOCLIENT_H

#include "asioservice.h"
#include "protoendpoint.h"

#include "server/asio/ssl_client.h"
#include "string/format.h"
#include "threads/thread.h"

class ProtoClient : public CppServer::Asio::SSLClient, public ProtoEndpoint
{
public:
    using CppServer::Asio::SSLClient::SSLClient;

    void DisconnectAndStop();

    bool connectReplyed();

    bool hasConnected(const std::string &ip) override;

    bool startHeartbeat();

protected:
    void onConnected() override;

    void onHandshaked() override;

    void onDisconnected() override;

    void onError(int error, const std::string &category, const std::string &message) override;

    // Protocol handlers
    void onReceive(const ::proto::DisconnectRequest &request) override;
    void onReceive(const ::proto::OriginMessage &response) override;
    void onReceive(const ::proto::MessageReject &reject) override;
    void onReceive(const ::proto::MessageNotify &notify) override;

    // Protocol implementation
    void onReceived(const void *buffer, size_t size) override;
    size_t onSend(const void *data, size_t size) override;

private:
    void handlePong(const std::string &remote);

    bool pingMessageStart();
    void pingTimerStop();

    void onHeartbeatTimeout(bool canceled);

private:
    std::atomic<bool> _stop { false };
    std::atomic<bool> _connect_replay { false };

    std::string _connected_host = { "" };
    // heartbeat: ping <-> pong
    std::shared_ptr<Timer> _ping_timer { nullptr };
    std::atomic<int> _nopong_count { 0 };
};

#endif // PROTOCLIENT_H
