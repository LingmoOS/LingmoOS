// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sessionworker.h"
#include "secureconfig.h"
#include "sessionmanager.h"

#include "common/log.h"
#include "common/commonutils.h"

#include <QHostInfo>
#include <QStandardPaths>
#include <QCoreApplication>
#include <QStorageInfo>

SessionWorker::SessionWorker(QObject *parent)
    : QObject(parent)
{
    // create own asio service
    _asioService = std::make_shared<AsioService>();
    if (!_asioService) {
        ELOG << "carete ASIO for session worker ERROR!";
        return;
    }
    _asioService->Start();

    QObject::connect(this, &SessionWorker::onRemoteDisconnected, this, &SessionWorker::handleRemoteDisconnected, Qt::QueuedConnection);
    QObject::connect(this, &SessionWorker::onRejectConnection, this, &SessionWorker::handleRejectConnection, Qt::QueuedConnection);
}

SessionWorker::~SessionWorker()
{
    _asioService->Stop();
}

void SessionWorker::onReceivedMessage(const proto::OriginMessage &request, proto::OriginMessage *response)
{
    // mark this rpc has received.
    response->id = request.id;
    response->mask = DO_SUCCESS;

    if (request.json_msg.empty()) {
        DLOG << "empty json message: ";
        return;
    }
#ifdef QT_DEBUG
    DLOG << "onReceivedMessage mask=" << request.mask << " json_msg: " << request.json_msg << std::endl;
#endif
    // Frist: 解析响应数据
    picojson::value v;
    std::string err = picojson::parse(v, request.json_msg);
    if (!err.empty()) {
        DLOG << "Failed to parse JSON data: " << err;
        return;
    }

    // if these exten mssages are handled, return
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
    case REQ_LOGIN: {
        LoginMessage req, res;
        req.from_json(v);
        DLOG << "Login: " << req.name << " " << req.auth;

        QString nice = QString::fromStdString(req.name);
        QByteArray pinByte = QByteArray::fromStdString(req.auth);
        QString dePin = QString::fromUtf8(QByteArray::fromBase64(pinByte));
        if (dePin == _savedPin) {
            res.auth = "thatsgood";
            emit onConnectChanged(LOGIN_SUCCESS, nice);
        } else {
            // return empty auth token.
            res.auth = "";
            emit onConnectChanged(LOGIN_DENIED, nice);

            emit onRejectConnection();
        }

        res.name = deepin_cross::CommonUitls::getFirstIp();
        response->json_msg = res.as_json().serialize();
        return;
    }
    break;
    case REQ_FREE_SPACE: {
        FreeSpaceMessage req, res;
        req.from_json(v);

        int remainSpace = 0;//TransferHelper::getRemainSize(); // xx G
        res.free = remainSpace;
        response->json_msg = res.as_json().serialize();
    }
    break;
    case REQ_TRANS_DATAS: {
        TransDataMessage req, res;
        req.from_json(v);

        QString endpoint = QString::fromStdString(req.endpoint);
        QStringList nameList;
        for (auto name : req.names) {
            nameList.append(QString::fromStdString(name));
        }

        res.id = req.id;
        res.names = req.names;
        res.flag = true;
        res.size = 0;//TransferHelper::getRemainSize();
        response->json_msg = res.as_json().serialize();

        emit onTransData(endpoint, nameList);

        uint64_t total = req.size;
        if (total > 0) {
            QString oneName = nameList.join(";");
            emit onTransCount(oneName, total);
        }
        return;
    }
    break;
    case REQ_TRANS_CANCLE: {
        TransCancelMessage req, res;
        req.from_json(v);

        DLOG << "recv cancel id: " << req.id << " " << req.reason;

        res.id = req.id;
        res.name = req.name;
        res.reason = "";
        response->json_msg = res.as_json().serialize();

        QString jobid = QString::fromStdString(req.id);
        QString reason = QString::fromStdString(req.reason);
        emit onCancelJob(jobid, reason);
        return;
    }
    break;
    case CAST_INFO: {
    }
    break;
    case INFO_TRANS_COUNT: {
        TransDataMessage req, res;
        req.from_json(v);

        QStringList nameList;
        for (auto name : req.names) {
            nameList.append(QString::fromStdString(name));
        }
        QString oneName = nameList.join(";");
        uint64_t total = req.size;

        res.id = req.id;
        res.names = req.names;
        res.flag = req.flag;
        res.size = total;
        response->json_msg = res.as_json().serialize();

        emit onTransCount(oneName, total);
        return;
    }
    break;
    default:
        DLOG << "unkown type: " << type;
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
    QString addr = QString::fromStdString(msg);

    switch (state) {
    case RPC_CONNECTED: {
        _connectedAddress = addr;
        DLOG << "connected remote: " << msg;
        _tryConnect = true;
        result = true;
    }
    break;
    case RPC_DISCONNECTED: {
        if (addr.isEmpty()) {
            DLOG << "disconnect with NULL, retry? " << _tryConnect;
            return _tryConnect;
        } else {
            DLOG << "disconnected remote: " << msg;
            emit onRemoteDisconnected(addr);
        }
    }
    break;
    case RPC_ERROR: {
        // code = 110: timeout, unabled ping
        DLOG << "error remote code: " << msg;
        int code = std::stoi(msg);
        if (asio::error::host_unreachable == code
            || asio::error::timed_out == code) {
            DLOG << "ping failed or timeout: " << msg;
            emit onConnectChanged(code, addr);
            return false;
        }
    }
    break;
    case RPC_PINGOUT: {
        // receive pong timeout
        DLOG << "timeout remote: " << msg;
        emit onRemoteDisconnected(addr);
    }
    break;
    default:
        DLOG << "other handling CONNECTING or DISCONNECTING: " << msg;
        break;
    }

    emit onConnectChanged(state, addr);

    return result;
}

void SessionWorker::setExtMessageHandler(ExtenMessageHandler cb)
{
    _extMsghandler = std::move(cb);
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
        ELOG << "Fail to start local listen:" << port;
        return false;
    }

    return true;
}

bool SessionWorker::netTouch(QString &address, int port)
{
    bool hasConnected = false;
    if (_client && _client->hasConnected(address.toStdString())) {
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

QString SessionWorker::sendRequest(const QString &target, const proto::OriginMessage &request)
{
    QString jsonContent = "";

    if (_client && _client->hasConnected(target.toStdString())) {
        auto res = _client->syncRequest(target.toStdString(), request);
        jsonContent = QString::fromStdString(res.json_msg);
        return jsonContent;
    }

    if (_server && _server->hasConnected(target.toStdString())) {
        auto res = _server->syncRequest(target.toStdString(), request);
        jsonContent = QString::fromStdString(res.json_msg);
        return jsonContent;
    }

    WLOG << "Not found connected session for: " << target.toStdString();
    return jsonContent;
}

bool SessionWorker::sendAsyncRequest(const QString &target, const proto::OriginMessage &request)
{
    if (target.isEmpty()) {
        ELOG << "empty target ip!!!";
        return false;
    }

    // Sleep for release something
    CppCommon::Thread::Yield();
    CppCommon::Thread::Sleep(1);

    std::string ip = target.toStdString();
    if (doAsyncRequest(_client.get(), ip, request)) {
        return true;
    }

    if (doAsyncRequest(_server.get(), ip, request)) {
        return true;
    }

    return false;
}

void SessionWorker::updatePincode(QString code)
{
    _savedPin = code;
}

void SessionWorker::updateLogin(QString ip, bool logined)
{
    _login_hosts.insert(ip, logined);
    if (_client)
        _client->startHeartbeat();
}

bool SessionWorker::isClientLogin(QString &ip)
{
    bool foundValue = false;
    bool hasConnected = false;
    auto it = _login_hosts.find(ip);
    if (it != _login_hosts.end()) {
        foundValue = it.value();
    }

    if (_client && _client->hasConnected(ip.toStdString())) {
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

bool SessionWorker::connect(QString &address, int port)
{
    auto context = SecureConfig::clientContext();

    if (!_client) {
        _client = std::make_shared<ProtoClient>(_asioService, context, address.toStdString(), port);

        auto self(this->shared_from_this());
        _client->setCallbacks(self);
    } else {
        if (_connectedAddress.compare(address) == 0) {
            LOG << "This target has been conntectd: " << address.toStdString();
            return _client->IsConnected() ? true : _client->ConnectAsync();
        } else {
            // different target, create new connection.
            _client->DisconnectAndStop();
            _client = std::make_shared<ProtoClient>(_asioService, context, address.toStdString(), port);

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
bool SessionWorker::doAsyncRequest(T *endpoint, const std::string& target, const proto::OriginMessage &request)
{
    if (endpoint && endpoint->hasConnected(target)) {
        endpoint->asyncRequestWithHandler(target, request, [this](int32_t type, const std::string &response) {
            QString res = QString::fromStdString(response);
            emit onRpcResult(type, res);
        });
        return true;
    }
    return false;
}


void SessionWorker::handleRemoteDisconnected(const QString &remote)
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
