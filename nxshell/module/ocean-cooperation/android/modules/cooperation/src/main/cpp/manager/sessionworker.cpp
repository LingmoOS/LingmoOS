// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sessionworker.h"
#include "secureconfig.h"
#include "sessionmanager.h"
#include "log.h"

#include <memory>
#include <vector>
#include <string>
#include <sstream>

SessionWorker::SessionWorker()
{
    // create own asio service
    _asioService = std::make_shared<AsioService>();
    if (!_asioService) {
        ELOG("create ASIO for session worker ERROR!");
        return;
    }
    _asioService->Start();
}

SessionWorker::~SessionWorker()
{
    _asioService->Stop();
}

// 其他方法省略...

void SessionWorker::onReceivedMessage(const proto::OriginMessage &request, proto::OriginMessage *response)
{
    response->id = request.id;
    response->mask = DO_SUCCESS;

    if (request.json_msg.empty()) {
        DLOG("empty json message: ");
        return;
    }

#ifdef DEBUG
    DLOG("onReceivedMessage mask= %d json_msg: %s", request.mask, request.json_msg);
#endif

    picojson::value v;
    std::string err = picojson::parse(v, request.json_msg);
    if (!err.empty()) {
        DLOG("Failed to parse JSON data: %s", err.c_str());
        return;
    }

    if (_extMsghandler) {
        std::string res_json;
        bool handled = _extMsghandler(request.mask, v, &res_json);
        if (handled) {
            response->json_msg = res_json;
            return;
        }
    }

    int type = request.mask;
    switch (type) {
    case REQ_TRANS_DATAS: {
        TransDataMessage req, res;
        req.from_json(v);

        std::string endpoint = req.endpoint;
        std::vector<std::string> nameList(req.names.begin(), req.names.end());

        res.id = req.id;
        res.names = req.names;
        res.flag = true;
        res.size = 0; // TransferHelper::getRemainSize();
        response->json_msg = res.as_json().serialize();

        // 处理 onTransData 事件（需要自己定义实现） 
        onTransData(endpoint, nameList);

        uint64_t total = req.size;
        if (total > 0) {
            std::ostringstream oss;
            for (size_t i = 0; i < nameList.size(); ++i) {
                oss << nameList[i];
                if (i < nameList.size() - 1) {
                    oss << ";";
                }
            }
            std::string oneName = oss.str();
            onTransCount(oneName, total); // 处理 onTransCount 事件
        }
        return;
    }
    break;

    case REQ_TRANS_CANCLE: {
        TransCancelMessage req, res;
        req.from_json(v);

        DLOG("recv cancel id: %s reason: %s", req.id.c_str(), req.reason.c_str() );

        res.id = req.id;
        res.name = req.name;
        res.reason = "";
        response->json_msg = res.as_json().serialize();

        std::string jobid = req.id;
        std::string reason = req.reason;
        onCancelJob(jobid, reason); // 处理 onCancelJob 事件
        return;
    }
    break;

    case CAST_INFO: {
        // 处理 CAST_INFO 
    }
    break;

    case INFO_TRANS_COUNT: {
        TransDataMessage req, res;
        req.from_json(v);

        std::vector<std::string> nameList(req.names.begin(), req.names.end());
        std::ostringstream oss;
        for (size_t i = 0; i < nameList.size(); ++i) {
            oss << nameList[i];
            if (i < nameList.size() - 1) {
                oss << ";";
            }
        }
        std::string oneName = oss.str();
        uint64_t total = req.size;

        res.id = req.id;
        res.names = req.names;
        res.flag = req.flag;
        res.size = total;
        response->json_msg = res.as_json().serialize();

        onTransCount(oneName, total); // 处理 onTransCount 事件
        return;
    }
    break;

    default:
        DLOG("unknown type: %d", type);
        break;
    }
}

bool SessionWorker::onStateChanged(int state, std::string &msg)
{
//    RPC_PINGOUT = -3,
//    RPC_ERROR = -2,
//    RPC_DISCONNECTED = -1,
//    RPC_DISCONNECTING = 0,
//    RPC_CONNECTING = 1,
//    RPC_CONNECTED = 2,
    bool result = false;
    std::string addr = std::string(msg);

    switch (state) {
    case RPC_CONNECTED: {
        _connectedAddress = addr;
        DLOG("connected remote: %s", msg.c_str());
        _tryConnect = true;
        result = true;
    }
    break;
    case RPC_DISCONNECTED: {
        if (addr.empty()) {
            DLOG("disconnect with NULL, retry? %d",_tryConnect);
            return _tryConnect;
        } else {
            DLOG("disconnected remote: %s", msg.c_str());
            handleRemoteDisconnected(msg.c_str());
        }
    }
    break;
    case RPC_ERROR: {
        // code = 110: timeout, unabled ping
        DLOG("error remote code: %s", msg.c_str());
        int code = std::stoi(msg);
        if (asio::error::host_unreachable == code
            || asio::error::timed_out == code) {
            DLOG("ping failed or timeout: %s", msg.c_str());
            // emit onConnectChanged(code, addr);
            _callback->onConnectChanged(code, addr);
            return false;
        }
    }
    break;
    case RPC_PINGOUT: {
        // receive pong timeout
        DLOG("timeout remote: %s", msg.c_str());
        // emit onRemoteDisconnected(addr);
    }
    break;
    default:
        DLOG("other handling CONNECTING or DISCONNECTING: %s", msg.c_str());
        break;
    }

    // onConnectChanged(state, addr);
    _callback->onConnectChanged(state, addr);

    return result;
}

void SessionWorker::setExtMessageHandler(ExtenMessageHandler cb)
{
    _extMsghandler = std::move(cb);
}

void SessionWorker::setCallbacks(std::shared_ptr<SessionCallback> cb)
{
    _callback = cb;
}

void SessionWorker::stop()
{
    if (_server) {
        // Stop the server
        _server->Stop();
    }

    if (_client) {
        _client->DisconnectAndStop();
    }
}

bool SessionWorker::startListen(int port)
{
    if (!listen(port)) {
        ELOG("Fail to start local listen: %d", port);
        return false;
    }

    return true;
}

bool SessionWorker::netTouch(const std::string &address, int port)
{
    bool hasConnected = false;
    if (_client && _client->hasConnected(address)) {
        hasConnected = _client->IsConnected();
    }

    if (hasConnected)
        return true;

    return connect(address, port);
}

void SessionWorker::disconnectRemote()
{
    if (!_client) return;

    _client->DisconnectAsync();
}

std::string SessionWorker::sendRequest(const std::string &target, const proto::OriginMessage &request)
{
    std::string jsonContent = "";

    if (_client && _client->hasConnected(target)) {
        auto res = _client->syncRequest(target, request);
        jsonContent = res.json_msg;
        return jsonContent;
    }

    if (_server && _server->hasConnected(target)) {
        auto res = _server->syncRequest(target, request);
        jsonContent = res.json_msg;
        return jsonContent;
    }

    WLOG("Not found connected session for: %s", target.c_str());
    return jsonContent;
}

bool SessionWorker::sendAsyncRequest(const std::string &target, const proto::OriginMessage &request)
{
    if (target.empty()) {
        ELOG("empty target ip!!!");
        return false;
    }

    // Sleep for release something
    CppCommon::Thread::Yield();
    CppCommon::Thread::Sleep(1);

    std::string ip = target;
    if (doAsyncRequest(_client.get(), ip, request)) {
        return true;
    }

    if (doAsyncRequest(_server.get(), ip, request)) {
        return true;
    }

    return false;
}

void SessionWorker::updatePincode(const std::string &code)
{
    _savedPin = code;
}

void SessionWorker::updateLogin(const std::string &ip, bool logined)
{
    _login_hosts.emplace(ip, logined);
    if (_client)
        _client->startHeartbeat();
}

bool SessionWorker::isClientLogin(const std::string &ip)
{
    bool foundValue = false;
    bool hasConnected = false;
    auto it = _login_hosts.find(ip);
    if (it != _login_hosts.end()) {
        foundValue = it->second;
    }

    if (_client && _client->hasConnected(ip)) {
        hasConnected = _client->IsConnected();
    }

    return foundValue && hasConnected;
}


bool SessionWorker::listen(int port)
{
    // Create a new proto protocol server
    if (!_server) {
        auto context = SecureConfig::serverContext();

        _server = std::make_shared<ProtoServer>(_asioService, context, port);
        _server->SetupReuseAddress(true);
        _server->SetupReusePort(true);

        auto self(this->shared_from_this());
        _server->setCallbacks(self);
    }

    // Start the server
    return _server->Start();
}

bool SessionWorker::connect(const std::string &address, int port)
{
    auto context = SecureConfig::clientContext();

    if (!_client) {
        _client = std::make_shared<ProtoClient>(_asioService, context, address, port);

        auto self(this->shared_from_this());
        _client->setCallbacks(self);
    } else {
        if (_connectedAddress.compare(address) == 0) {
            DLOG("This target has been conntectd: %s", address.c_str());
            return _client->IsConnected() ? true : _client->ConnectAsync();
        } else {
            // different target, create new connection.
            _client->DisconnectAndStop();
            _client = std::make_shared<ProtoClient>(_asioService, context, address, port);

            auto self(this->shared_from_this());
            _client->setCallbacks(self);
        }
    }

    int wait_cout = 0;
    _tryConnect = false;
    _client->ConnectAsync();
    // wait until has reply, total 1s timeout
    while (!_client->connectReplyed()) {
        if (wait_cout > 2000)
            break;
        CppCommon::Thread::Sleep(1);
        CppCommon::Thread::Yield();
        wait_cout++;
    };

    return _client->IsConnected();
}

template<typename T>
bool SessionWorker::doAsyncRequest(T *endpoint, const std::string &target, const proto::OriginMessage &request)
{
    if (endpoint && endpoint->hasConnected(target)) {
        endpoint->asyncRequestWithHandler(target, request, [this](int32_t type, const std::string &response) {
//            emit onRpcResult(type, response);
            _callback->onRpcResult(type, response);
        });
        return true;
    }
    return false;
}


void SessionWorker::handleRemoteDisconnected(const std::string &remote)
{
    if (_connectedAddress == remote) {
        _connectedAddress = "";
    }
    auto it = _login_hosts.find(remote);
    if (it != _login_hosts.end()) {
        _login_hosts.erase(it);
    }
}

void SessionWorker::handleRejectConnection()
{
    if (_server) {
        // Send disconnect
        _server->sendDisRequest();
    }
}

void SessionWorker::onTransData(const std::string &endpoint, const std::vector<std::string> &nameList)
{
    // 处理相应传输数据事件的实现
}

void SessionWorker::onTransCount(const std::string &names, uint64_t size)
{
    // 处理传输计数事件的实现
}

void SessionWorker::onCancelJob(const std::string &jobid, const std::string &reason)
{
    // 处理取消作业事件的实现
}

void SessionWorker::onTransResult(const std::string &jobid, const std::string &result)
{

}

