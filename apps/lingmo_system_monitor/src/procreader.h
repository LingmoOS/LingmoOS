// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "diskreader.h"
#include "gpureader.h"
#include "metrics.h"
#include "networkreader.h"
#include "processrecord.h"

#include <chrono>
#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

struct CpuCounters
{
    uint64_t total = 0;
    uint64_t idle = 0;
};

struct Snapshot
{
    bool available = false;
    bool baseline = false;
    std::string error;
    Metrics metrics;
    std::vector<ProcessRecord> processes;
};

class ProcReader
{
public:
    explicit ProcReader(std::string procRoot = "/proc", std::string sysRoot = "/sys");

    Snapshot sample();
    static CpuCounters parseCpu(std::string_view line);
    static MemoryMetrics parseMemory(std::string_view contents);
    static bool parseProcess(std::string_view contents, ProcessRecord* process, int64_t pageSize);
    static bool parseUid(std::string_view status, uint32_t* uid);
    static std::string commandArguments(std::string_view commandLine, const std::string& executable);
    static std::string applicationProgram(std::string_view commandLine);
    static std::string helperType(std::string_view commandLine);
    static std::string helperName(const std::string& type);
    static std::string scopeApplication(std::string_view cgroup);
    static bool parsePss(std::string_view rollup, uint64_t* pss);
    static double cpuUsage(const CpuCounters& current, const CpuCounters& previous);
    static std::string stateName(const std::string& state);

private:
    void readIdentity(const std::string& path, ProcessRecord* process) const;
    std::string userName(uint32_t uid);

    std::string m_procRoot;
    std::chrono::steady_clock::time_point m_clock = std::chrono::steady_clock::now();
    int64_t m_previousTime = 0;
    bool m_hasPreviousSample = false;
    int64_t m_pageSize = 4096;
    uint32_t m_uid = 0;
    std::unordered_map<std::string, CpuCounters> m_previousCpu;
    std::unordered_map<int, ProcessRecord> m_previousProcesses;
    std::unordered_map<uint32_t, std::string> m_users;
    std::string m_cpuName;
    NetworkReader m_networks;
    DiskReader m_disks;
    GpuReader m_gpus;
};
