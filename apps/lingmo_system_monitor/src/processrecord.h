// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>
#include <string>
#include <vector>

struct ProcessRecord
{
    int pid = 0;
    int parentPid = 0;
    uint32_t uid = 0;
    std::string name;
    std::string displayName;
    std::string iconName;
    std::string user;
    std::string command;
    std::string executable;
    std::string program;
    std::string scopeId;
    // Chromium's helper process type, such as renderer; empty for other processes.
    std::string helperType;
    std::string appId;
    std::string state;
    int threads = 0;
    int nice = 0;
    uint64_t startTicks = 0;
    uint64_t cpuTicks = 0;
    uint64_t rss = 0;
    // PSS when it could be read, resident memory otherwise.
    uint64_t memory = 0;
    uint64_t readBytes = 0;
    uint64_t writeBytes = 0;
    uint64_t pss = 0;
    int64_t pssTime = 0;
    bool pssAvailable = false;
    bool ioAvailable = false;
    bool kernelThread = false;
    bool scopeSettled = false;
    double cpu = 0;
    double readRate = 0;
    double writeRate = 0;
    // Set on application groups only: every process the application owns.
    std::vector<ProcessRecord> members;
};
