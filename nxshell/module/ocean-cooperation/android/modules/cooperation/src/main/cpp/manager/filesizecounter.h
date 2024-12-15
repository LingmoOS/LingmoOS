// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILESIZECOUNTER_H
#define FILESIZECOUNTER_H

#include <thread>
#include <string>
#include <vector>
#include <functional>

class FileSizeCounter {
public:
    explicit FileSizeCounter();

    // 计数文件大小
    uint64_t countFiles(const std::string &targetIp, const std::vector<std::string> &paths);

    // 完成计数后要执行的回调
    void setOnCountFinishCallback(std::function<void(const std::string&, const std::vector<std::string>&, uint64_t)> callback);

private:
    void run();
    void countFilesInDir(const std::string &path);

    std::vector<std::string> _paths;
    std::string _targetIp;
    uint64_t _totalSize {0};
    std::function<void(const std::string&, const std::vector<std::string>&, uint64_t)> _onCountFinishCallback;
};

#endif // FILESIZECOUNTER_H
