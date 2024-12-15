// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PROTOSERVER_H
#define PROTOSERVER_H

#include "asioservice.h"
#include "protoendpoint.h"

#include "server/asio/ssl_server.h"

class ProtoServer : public CppServer::Asio::SSLServer, public ProtoEndpoint
{
public:
    using CppServer::Asio::SSLServer::SSLServer;

    bool hasConnected(const std::string &ip) override;

protected:
    std::shared_ptr<CppServer::Asio::SSLSession> CreateSession(const std::shared_ptr<CppServer::Asio::SSLServer> &server) override;

protected:
    void onError(int error, const std::string &category, const std::string &message) override;

    void onConnected(std::shared_ptr<CppServer::Asio::SSLSession>& session) override;

    void onDisconnected(std::shared_ptr<CppServer::Asio::SSLSession>& session) override;

    // Protocol implementation
    size_t onSend(const void *data, size_t size) override;

private:
    bool startHeartbeat();

    void handlePing(const std::string &remote);

    void onHeartbeatTimeout(bool canceled);

private:
    // <ip, sessionid>
    std::shared_mutex _sessionids_lock;
    std::map<std::string, CppCommon::UUID> _session_ids;

    // heartbeat: ping <-> pong
    std::shared_ptr<Timer> _ping_timer { nullptr };
    // <ip, pinged>
    std::map<std::string, std::atomic<int>> _ping_remotes;
};

#endif // PROTOSERVER_H
