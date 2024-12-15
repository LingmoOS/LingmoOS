// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TRANSFERWORKER_H
#define TRANSFERWORKER_H

#include "session/asioservice.h"
#include "httpweb/fileserver.h"
#include "httpweb/fileclient.h"

#include <iostream>
#include <thread>
#include <atomic>
#include <vector>
#include <string>
#include <chrono>
#include <functional>
#include <mutex>

class TransferWorker : public ProgressCallInterface
{
    struct file_stats_s {
        std::string path;
        uint64_t total {0};   // Total size
        std::atomic<uint64_t> secsize {0};   // Transfer size per second
    };

public:
    explicit TransferWorker(std::string id);

    bool onProgress(uint64_t size) override;
    void onWebChanged(int state, std::string msg = "", uint64_t size = 0) override;

    void stop();
    bool tryStartSend(const std::vector<std::string>& paths, int port, std::vector<std::string>* nameVector, std::string* token);
    bool tryStartReceive(const std::vector<std::string>& names, const std::string& ip, int port, const std::string& token, const std::string& dirname);

    bool isSyncing();
    void setEveryFileNotify(bool every);
    bool isServe();

private:
    void handleTimerTick(bool stop);
    void doCalculateSpeed();
    void sendTranEndNotify();
    
    bool startWeb(int port);
    bool startGet(const std::string &address, int port);

    std::shared_ptr<AsioService> _asioService;
    std::shared_ptr<FileServer> _file_server { nullptr };
    std::shared_ptr<FileClient> _file_client { nullptr };

    std::atomic<bool> _canceled { false };
    bool _singleFile { false }; // Send single file
    bool _everyNotify { false }; // Notify every file
    std::string _recvPath { "" }; // Files receive path
    std::string _bindId; // Bind target IP, which is the worker's ID
    file_stats_s _status;
    int _noDataCount = 0;

    std::mutex _mutex; // Mutex for thread safety
};

#endif // TRANSFERWORKER_H
