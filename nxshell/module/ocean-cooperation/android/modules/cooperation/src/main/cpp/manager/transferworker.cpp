// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "transferworker.h"
#include "sessionmanager.h"
#include "secureconfig.h"

#include "common/log.h"
#include "common/constant.h"

#include <chrono>
#include <thread>

TransferWorker::TransferWorker(std::string id)
    : _bindId(id)
{
    // Create own asio service
    _asioService = std::make_shared<AsioService>();
    if (!_asioService) {
        std::cerr << "Create ASIO for transfer worker ERROR!" << std::endl;
    }
    _asioService->Start();
}

bool TransferWorker::onProgress(uint64_t size)
{
    _status.secsize.fetch_add(size);
    return _canceled;
}

void TransferWorker::onWebChanged(int state, std::string msg, uint64_t size)
{
    if (state < 1) {
        handleTimerTick(true);
        std::cerr << "Error: " << msg << std::endl;
        return;
    }

    switch(state) {
    case 1: // WEB_CONNECTED
        break;
    case 2: // WEB_TRANS_START
        std::cout << "Notify whole web transfer start!" << std::endl;
        handleTimerTick(false);
        break;
    case 3: // WEB_TRANS_FINISH
        std::cout << "Notify whole web transfer finished!" << std::endl;
        sendTranEndNotify();
        break;
    case 4: // WEB_INDEX_BEGIN
        std::cout << "Notify web index: " << msg << std::endl;
        break;
    case 5: // WEB_FILE_BEGIN
        _status.path = msg;
        _status.total = size;
        if (_everyNotify) {
            std::cout << "Notify file begin: " << msg << std::endl;
        }
        break;
    case 6: // WEB_FILE_END
        _status.path = msg;
        _status.total = size;
        if (_everyNotify) {
            std::cout << "Notify file end: " << msg << std::endl;
        }
        break;
    }
}

void TransferWorker::stop()
{
    _canceled = true;
    handleTimerTick(true);

    if (_file_server) {
        _file_server->clearBind();
        _file_server->stop();
    }

    if (_file_client) {
        _file_client->stop();
    }
}

bool TransferWorker::tryStartSend(const std::vector<std::string>& paths, int port, std::vector<std::string>* nameVector, std::string* token)
{
    _singleFile = false; // reset for send files
    _recvPath = "";

    if (!startWeb(port)) {
        std::cerr << "Try to start web server failed!!!" << std::endl;
        return false;
    }

    _file_server->clearBind();
    for (const auto& path : paths) {
        std::string name = path; // Use the file name directly for simplicity
        nameVector->push_back(name);
        try {
            _file_server->webBind(name, path);
        } catch (...) {
            std::cerr << "Web bind (" << name << ") throw exception." << std::endl;
        }
        
        if (paths.size() == 1) {
            _singleFile = true;
        }
    }

    *token = _file_server->genToken(""); // Replace with actual token generation logic

    _canceled = false;
    return true;
}

bool TransferWorker::tryStartReceive(const std::vector<std::string>& names, const std::string& ip, int port, const std::string& token, const std::string& dirname)
{
    _singleFile = false; // reset for send files
    _recvPath = dirname;

    if (!startGet(ip, port)) {
        std::cerr << "Try to create HTTP getter failed!!!" << std::endl;
        return false;
    }

    _file_client->setConfig(token, dirname);
    std::vector<std::string> webs = _file_client->parseWeb(token);
    _file_client->startFileDownload(webs);

    _canceled = false;
    return true;
}

bool TransferWorker::isSyncing()
{
    return !_canceled;
}

void TransferWorker::setEveryFileNotify(bool every)
{
    _everyNotify = every;
}

bool TransferWorker::isServe()
{
    return _recvPath.empty();
}

void TransferWorker::handleTimerTick(bool stop)
{
    // Here we can control the behavior similar to QTimer
    if (stop) {
        // Stop the speed calculation
    } else {
        // Start the speed calculation in a separate thread
        std::thread([this]() {
            while (!_canceled) {
                doCalculateSpeed();
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        }).detach();
    }
}

void TransferWorker::doCalculateSpeed()
{
    int64_t bytesize = _status.secsize.load();
    _status.secsize.store(0); // reset every second

    if (bytesize > 0) {
        _noDataCount = 0;
    } else {
        _noDataCount++;
    }

    double speed = static_cast<double>(bytesize) / (1024 * 1024); // Calculate speed in MB/s
    std::cout << "Transfer speed: " << speed << " M/s" << std::endl;
    std::cout << "Path: " << _status.path << ", Bytes: " << bytesize << std::endl;
}

void TransferWorker::sendTranEndNotify()
{
    handleTimerTick(true);
    std::cout << "Transfer complete: " << _recvPath << std::endl;
}

bool TransferWorker::startWeb(int port)
{
    if (!_file_server) {
        auto context = SecureConfig::serverContext();
        _file_server = std::make_shared<FileServer>(_asioService, context, port);
    }
    return _file_server->start();
}

bool TransferWorker::startGet(const std::string &address, int port)
{
    if (!_file_client) {
        auto context = SecureConfig::clientContext();
        _file_client = std::make_shared<FileClient>(_asioService, context, address, port);
    }
    return true;
}
