// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "protoclient.h"

void ProtoClient::DisconnectAndStop()
{
    _stop = true;
    _connect_replay = false;
    DisconnectAsync();
    while (IsConnected())
        CppCommon::Thread::Yield();
}

bool ProtoClient::hasConnected(const std::string &ip)
{
    return ip == _connected_host;
}

bool ProtoClient::startHeartbeat()
{
    if (!_ping_timer) {
        _ping_timer = std::make_unique<Timer>(service());

        std::function<void(bool)> _action = std::bind(&ProtoClient::onHeartbeatTimeout, this, std::placeholders::_1);
        _ping_timer->Setup(_action);
    }

    return pingMessageStart();
}

void ProtoClient::handlePong(const std::string &remote)
{
    // std::cout << "client pong: " << remote << std::endl;
    _connected_host = remote;
    _nopong_count.store(0);
}

bool ProtoClient::pingMessageStart()
{
    if (_connected_host.empty() || !IsHandshaked())
        return false;

    proto::MessageNotify ping;
    ping.notification = "ping";
    send(ping);

    _ping_timer->Setup(CppCommon::Timespan::seconds(HEARTBEAT_INTERVAL));
    return _ping_timer->WaitAsync();
}

void ProtoClient::pingTimerStop()
{
    _connected_host = "";
    if (_ping_timer) {
        _ping_timer->Cancel();
    }
}

void ProtoClient::onHeartbeatTimeout(bool canceled)
{
    // std::cout << "Cient timer handleHeartbeat!" << std::endl;
    if (!canceled) {
        auto count = _nopong_count.load();
        if (count < 3) {
            _nopong_count.fetch_add(1);
            pingMessageStart();
        } else {
            // no pong more than 3 times
            if (_callbacks)
                _callbacks->onStateChanged(RPC_PINGOUT, _connected_host);
            pingTimerStop();
        }
    }
}

bool ProtoClient::connectReplyed()
{
    return _connect_replay;
}

void ProtoClient::onConnected()
{
    // std::cout << "Protocol client connected a new session with Id " << id() << " ip:" << address() << std::endl;
    // For SSL: must wait handshake completed, so move into handshaked
}

void ProtoClient::onHandshaked()
{
    // std::cout << "Proto SSL client handshaked a new session with Id " << id() << std::endl;
    _connect_replay = true;

    // Reset FBE protocol buffers
    reset();

    _connected_host = socket().remote_endpoint().address().to_string();
    if (_callbacks) {
        _callbacks->onStateChanged(RPC_CONNECTED, _connected_host);
    }
}

void ProtoClient::onDisconnected()
{
    // std::cout << "Protocol client disconnected a session with Id " << id() << std::endl;
    _connect_replay = true;

    bool retry = true;
    if (_callbacks) {
        //can not get the remote address if has not connected yet.
         retry = _callbacks->onStateChanged(RPC_DISCONNECTED, _connected_host);
    }
    pingTimerStop();

    if (retry) {
        // Wait for a while...
        CppCommon::Thread::Sleep(1000);

        // Try to connect again
        if (!_stop)
            ConnectAsync();
    }
}

void ProtoClient::onError(int error, const std::string &category, const std::string &message)
{
    // std::cout << "Protocol client caught an error with code " << error << " and category '" << category << "': " << message << std::endl;
    _connect_replay = true;
    if (_callbacks) {
        std::string err = std::to_string(error);
        _callbacks->onStateChanged(RPC_ERROR, err);
    }
}

// Protocol handlers
void ProtoClient::onReceive(const ::proto::DisconnectRequest &request)
{
    FinalClient::onReceive(request);
    //std::cout << "Received disconnect: " << request << std::endl;
    if (_callbacks) {
        std::string addr = socket().remote_endpoint().address().to_string();
        _callbacks->onStateChanged(RPC_DISCONNECTING, addr);
    }
    DisconnectAsync();
}

void ProtoClient::onReceive(const ::proto::OriginMessage &response)
{
    // notify response if the request from myself, request.get()
    if (_self_request.load(std::memory_order_relaxed)) {
        _self_request.store(false, std::memory_order_relaxed);
        FinalClient::onReceiveResponse(response);
        return;
    }

    // Send response
    proto::OriginMessage reply;

    if (_callbacks) {
        _callbacks->onReceivedMessage(response, &reply);
    } else {
        reply.id = response.id;
        reply.mask = response.mask;
        reply.json_msg = response.json_msg;
    }

    if (!reply.json_msg.empty())
        send(reply);
}

void ProtoClient::onReceive(const ::proto::MessageReject &reject)
{
    FinalClient::onReceive(reject);
    //std::cout << "Received reject: " << reject << std::endl;
}

void ProtoClient::onReceive(const ::proto::MessageNotify &notify)
{
    // FinalClient::onReceive(notify);
    // std::cout << "Received notify: " << notify << std::endl;

    // mark pinged
    auto remote = socket().remote_endpoint().address().to_string();
    handlePong(remote);
}

// Protocol implementation
void ProtoClient::onReceived(const void *buffer, size_t size)
{
    receive(buffer, size);
}

size_t ProtoClient::onSend(const void *data, size_t size)
{
    return SendAsync(data, size) ? size : 0;
}
