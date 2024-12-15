// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "filesizecounter.h"
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

FileSizeCounter::FileSizeCounter() {
}

uint64_t FileSizeCounter::countFiles(const std::string &targetIp, const std::vector<std::string> &paths) {
    _targetIp = targetIp;
    _paths.clear();
    uint64_t totalSize = 0;

    for (const auto &path : paths) {
        fs::path filePath(path);
        if (fs::is_directory(filePath)) {
            _paths = paths;
            std::thread(&FileSizeCounter::run, this).detach(); // 启动线程
            return 0; // 线程将异步处理
        } else if (fs::is_regular_file(filePath)) {
            totalSize += fs::file_size(filePath);
        }
    }

    return totalSize;
}

void FileSizeCounter::run() {
    _totalSize = 0;
    for (const auto &path : _paths) {
        countFilesInDir(path);
    }
    // 调用回调函数通知完成
    if (_onCountFinishCallback) {
        _onCountFinishCallback(_targetIp, _paths, _totalSize);
    }
}

void FileSizeCounter::countFilesInDir(const std::string &path) {
    fs::path dirPath(path);
    if (fs::is_regular_file(dirPath)) {
        _totalSize += fs::file_size(dirPath);
        return;
    }

    for (const auto &entry : fs::directory_iterator(dirPath)) {
        if (fs::is_directory(entry)) {
            countFilesInDir(entry.path().string()); // 递归遍历子目录
        } else {
            _totalSize += fs::file_size(entry); // 统计文件大小
        }
    }
}

void FileSizeCounter::setOnCountFinishCallback(std::function<void(const std::string&, const std::vector<std::string>&, uint64_t)> callback) {
    _onCountFinishCallback = std::move(callback);
}
