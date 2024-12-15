// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SESSIONMANAGER_H
#define SESSIONMANAGER_H

#include "sessionproto.h"
#include "common/exportglobal.h"

#include <string>
#include <map>
#include <memory>
#include <vector>
#include <functional>

class SessionWorker;
class TransferWorker;
class FileSizeCounter;

class EXPORT_API SessionManager: public SessionCallback {
public:
    SessionManager(const std::string &ip, int port);
    ~SessionManager();

    void onConnectChanged(int result, const std::string &reason) override;
    void onRpcResult(int type, const std::string &response) override;
    void setJavaCallback(CallBackHandler cb);

    void setSessionExtCallback(ExtenMessageHandler cb);
    void updatePin(const std::string &code);
    void setStorageRoot(const std::string &root);
    void updateSaveFolder(const std::string &folder);
    void updateNick(const std::string &nick);
    void updateLoginStatus(const std::string &ip, bool logined);

    void sessionListen();
    bool sessionPing(const std::string &ip, int port);
    int sessionConnect(const std::string &ip, int port, const std::string &password);
    void sessionDisconnect(const std::string &ip);
    void sendFiles(const std::string &ip, int port, const std::vector<std::string> &paths);
    void recvFiles(const std::string &ip, int port, const std::string &token, const std::vector<std::string> &names);
    void cancelSyncFile(const std::string &ip, const std::string &reason = "");
    void sendRpcRequest(const std::string &target, int type, const std::string &reqJson);


private:
    std::shared_ptr<TransferWorker> createTransWorker(const std::string &jobid);

    // 处理各种事件
    void handleTransData(const std::string &endpoint, const std::vector<std::string> &nameVector);
    void handleTransCount(const std::string &names, uint64_t size);
    void handleCancelTrans(const std::string &jobid, const std::string &reason);
    void handleFileCounted(const std::string &ip, const std::vector<std::string> &paths, uint64_t totalSize);
    void handleRpcResult(int32_t type, const std::string &response);
    void handleTransException(const std::string &jobid, const std::string &reason);

private:
    CallBackHandler _javaCallbackhandler { nullptr };

    std::shared_ptr<SessionWorker> _session_worker;
    std::shared_ptr<FileSizeCounter> _file_counter;
    std::map<std::string, std::shared_ptr<TransferWorker>> _trans_workers;

    std::string _save_root;
    std::string _save_dir;
    std::string _device_name;

    std::string serveIP;
    int         servePort;
};

#endif // SESSIONMANAGER_H
