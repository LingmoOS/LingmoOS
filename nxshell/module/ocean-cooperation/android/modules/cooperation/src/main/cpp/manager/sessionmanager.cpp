// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sessionmanager.h"
#include "sessionproto.h"
#include "sessionworker.h"
#include "transferworker.h"
#include "filesizecounter.h"
#include "log.h"
#include "utils.h"

#include <numeric>
#include <filesystem>

#include "string/encoding.h"

SessionManager::SessionManager(const std::string &ip, int port)
    : serveIP(ip)
    , servePort(port)
{
    _trans_workers.clear();
    // Create session and transfer worker
    _session_worker = std::make_shared<SessionWorker>();
    _file_counter = std::make_shared<FileSizeCounter>();

    // 注意：此处需要替换信号与槽的事件处理机制。
}

SessionManager::~SessionManager() {
    // TODO: 清理工作
}

void SessionManager::onConnectChanged(int result, const std::string &reason)
{
    DLOG("onConnectChanged: %d %s", result, reason.c_str());
    if (_javaCallbackhandler) {
        _javaCallbackhandler(CONN_CHANGED, result, reason);
    }
}

void SessionManager::onRpcResult(int type, const std::string &response)
{
    DLOG("onRpcResult: %d %s", type, response.c_str());
    if (_javaCallbackhandler) {
        _javaCallbackhandler(RPC_RESULT, type, response);
    }
}

void SessionManager::setJavaCallback(CallBackHandler cb)
{
    _javaCallbackhandler = std::move(cb);
    auto self(this->shared_from_this());
    _session_worker->setCallbacks(self);
}

void SessionManager::setSessionExtCallback(ExtenMessageHandler cb) {
    _session_worker->setExtMessageHandler(cb);
}

void SessionManager::updatePin(const std::string &code) {
    _session_worker->updatePincode(code);
}

void SessionManager::setStorageRoot(const std::string &root) {
    _save_root = root;
}

void SessionManager::updateSaveFolder(const std::string &folder) {
    if (_save_root.empty()) {
        _save_root = std::filesystem::current_path().string(); // 设置为当前工作目录
    }
    _save_dir = _save_root + "/";
    if (!folder.empty()) {
        _save_dir += folder + "/";
    }
}

void SessionManager::updateNick(const std::string &nick)
{
    _device_name = nick;
}

void SessionManager::updateLoginStatus(const std::string &ip, bool logined) {
    _session_worker->updateLogin(ip, logined);
}

void SessionManager::sessionListen() {
    bool success = _session_worker->startListen(servePort);
    if (!success) {
        ELOG("Fail to start listen: %d", servePort);
    }
}

bool SessionManager::sessionPing(const std::string &ip, int port) {
    DLOG("sessionPing: %s", ip.c_str());
    return _session_worker->netTouch(ip, port);
}

int SessionManager::sessionConnect(const std::string &ip, int port, const std::string &password) {
    DLOG("sessionConnect: %s", ip.c_str());
    if (_session_worker->isClientLogin(ip)){
        DLOG("isClientLogin: %s", ip.c_str());
        return 1;
    }
    if (!_session_worker->netTouch(ip, port)) {
        ELOG("Fail to touch remote: %s", ip.c_str());
        return -1;
    }
    DLOG("REQ_LOGIN: %s", ip.c_str());
    // base64 encode
    std::string pinString = CppCommon::Encoding::Base64Encode(password);

    LoginMessage req;
    req.name = serveIP + ":" + _device_name;
    req.auth = pinString;

    std::string jsonMsg = req.as_json().serialize();
    sendRpcRequest(ip, REQ_LOGIN, jsonMsg);

    return 0;
}

void SessionManager::sessionDisconnect(const std::string &ip) {
    DLOG("sessionDisconnect: %s", ip.c_str());
    _session_worker->disconnectRemote();
}

std::shared_ptr<TransferWorker> SessionManager::createTransWorker(const std::string &jobid) {
    auto newWorker = std::make_shared<TransferWorker>(jobid);
    // Store it in the map with the given jobid
    _trans_workers[jobid] = newWorker;
    return newWorker;
}

void SessionManager::sendFiles(const std::string &ip, int port, const std::vector<std::string> &paths) {
    std::vector<std::string> name_vector;
    std::string token;

    auto worker = createTransWorker(ip);
    bool success = worker->tryStartSend(paths, port, &name_vector, &token);
    if (!success) {
        ELOG("Fail to start size: %zu at: %d", paths.size(), port);
        return;
    }

    std::string localIp = serveIP;
    std::string accesstoken = token;
    std::string endpoint = localIp + ":" + std::to_string(port) + ":" + accesstoken;
    uint64_t total = _file_counter->countFiles(ip, paths);
    bool needCount = total == 0;

    TransDataMessage req;
    req.id = ip;
    req.names = name_vector;
    req.endpoint = endpoint;
    req.flag = needCount; // many folders
    req.size = total; // unknown size

    std::string jsonMsg = req.as_json().serialize();
    sendRpcRequest(ip, REQ_TRANS_DATAS, jsonMsg);

    if (total > 0) {
        std::string oneName = std::accumulate(paths.begin(), paths.end(), std::string(),
            [](const std::string &a, const std::string &b) {
                return a.empty() ? b : a + ";" + b;
            });
        handleTransCount(oneName, total);
    }
}

void SessionManager::recvFiles(const std::string &ip, int port, const std::string &token, const std::vector<std::string> &names) {
    auto worker = createTransWorker(ip);
    bool success = worker->tryStartReceive(names, ip, port, token, _save_dir);
    if (!success) {
        ELOG("Fail to recv name size: %zu at: %s", names.size(), ip.c_str());
    }
}

void SessionManager::cancelSyncFile(const std::string &ip, const std::string &reason) {
    DLOG("cancelSyncFile: %s", ip.c_str());

    TransCancelMessage req;
    req.id = serveIP;
    req.name = "all";
    req.reason = reason;

    std::string jsonMsg = req.as_json().serialize();
    sendRpcRequest(ip, REQ_TRANS_CANCLE, jsonMsg);

    handleCancelTrans(ip, reason);
}

void SessionManager::sendRpcRequest(const std::string &ip, int type, const std::string &reqJson) {
    proto::OriginMessage request;
    request.mask = type;
    request.json_msg = reqJson;

    _session_worker->sendAsyncRequest(ip, request);
}

void SessionManager::handleTransData(const std::string &endpoint, const std::vector<std::string> &nameVector) {
    auto parts = Utils::split(endpoint, ':'); // 使用自己的字符串拆分函数
    if (parts.size() == 3) {
        recvFiles(parts[0], std::stoi(parts[1]), parts[2], nameVector);
    } else {
        WLOG("endpoint format should be: ip:port:token");
    }
}

void SessionManager::handleTransCount(const std::string &names, uint64_t size) {
    // 对于传输计数的处理
    // 这里可以替换为您自己的处理逻辑
}

void SessionManager::handleCancelTrans(const std::string &jobid, const std::string &reason) {
    auto it = _trans_workers.find(jobid);
    if (it != _trans_workers.end()) {
        it->second->stop();
        _trans_workers.erase(it);
    }

    if (!reason.empty()) {
        // 异常处理，针对具体情况执行
    } else {
        // 用户取消的情况处理
    }
}

void SessionManager::handleFileCounted(const std::string &ip, const std::vector<std::string> &paths, uint64_t totalSize) {
    if (ip.empty()) {
        WLOG("empty target address for file counted.");
        return;
    }
    std::vector<std::string> nameVector;
    for (const auto &path : paths) {
        // 处理文件路径以获得文件名
        std::string name = std::filesystem::path(path).filename().string();
        nameVector.push_back(name);
    }

    // 使用 fmt::join 来连接路径
    std::string joinedPaths = Utils::join(paths, ";");

    TransDataMessage req;
    req.id = ip;
    req.names = nameVector;
    req.endpoint = "::";
    req.flag = false; // no need count
    req.size = totalSize;

    std::string jsonMsg = req.as_json().serialize();
    sendRpcRequest(ip, INFO_TRANS_COUNT, jsonMsg);

    handleTransCount(joinedPaths, totalSize); // 添加 join 函数合并路径
}

void SessionManager::handleRpcResult(int32_t type, const std::string &response) {
    // 对于 RPC 结果的处理
}

void SessionManager::handleTransException(const std::string &jobid, const std::string &reason) {
    cancelSyncFile(jobid, reason);
}
